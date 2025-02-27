// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "session/internal/keymap_factory.h"

#include <iterator>
#include <map>

#include "composer/key_parser.h"
#include "config/config_handler.h"
#include "protocol/commands.pb.h"
#include "protocol/config.pb.h"
#include "session/internal/keymap.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"

namespace mozc {
namespace keymap {

class TestKeyMapFactoryProxy {
 public:
  static void Clear() {
    KeyMapFactory::KeyMapManagerMap &keymaps = *KeyMapFactory::GetKeyMaps();
    for (KeyMapFactory::KeyMapManagerMap::iterator iter = keymaps.begin();
         iter != keymaps.end(); ++iter) {
      KeyMapFactory::GetPool()->Release(iter->second);
    }

    keymaps.clear();
  }
};

namespace {

class KeyMapFactoryTest : public testing::Test {
 protected:
  void SetUp() override { TestKeyMapFactoryProxy::Clear(); }

  void TearDown() override { TestKeyMapFactoryProxy::Clear(); }
};

}  // namespace

TEST_F(KeyMapFactoryTest, KeyMapFactoryTest) {
  {  // KeyMapFactory returns correct instance.
    KeyMapManager *atok = KeyMapFactory::GetKeyMapManager(config::Config::ATOK);
    KeyMapManager *msime =
        KeyMapFactory::GetKeyMapManager(config::Config::MSIME);
    KeyMapManager *kotoeri =
        KeyMapFactory::GetKeyMapManager(config::Config::KOTOERI);
    KeyMapManager *custom =
        KeyMapFactory::GetKeyMapManager(config::Config::CUSTOM);
    KeyMapManager *mobile =
        KeyMapFactory::GetKeyMapManager(config::Config::MOBILE);

    // GetKeyMapFactory() returns the same instance when argument is the same
    EXPECT_EQ(atok, KeyMapFactory::GetKeyMapManager(config::Config::ATOK));
    EXPECT_EQ(msime, KeyMapFactory::GetKeyMapManager(config::Config::MSIME));
    EXPECT_EQ(kotoeri,
              KeyMapFactory::GetKeyMapManager(config::Config::KOTOERI));
    EXPECT_EQ(custom, KeyMapFactory::GetKeyMapManager(config::Config::CUSTOM));
    EXPECT_EQ(mobile, KeyMapFactory::GetKeyMapManager(config::Config::MOBILE));

    // GetKeyMapFactory() does not return the same instance
    // when argument is not the same
    KeyMapManager *keymap_array[] = {
        atok, msime, kotoeri, custom, mobile,
    };
    const int array_size = std::size(keymap_array);
    for (int i = 0; i < array_size; ++i) {
      for (int j = 0; j < array_size; ++j) {
        if (i != j) {
          EXPECT_NE(keymap_array[i], keymap_array[j])
              << "current_index = (" << i << "," << j << ")";
        }
      }
    }
  }
}

TEST_F(KeyMapFactoryTest, ReloadWhenGetInstance) {
  commands::KeyEvent key;
  key.set_special_key(commands::KeyEvent::SPACE);
  config::Config config;

  {  // ConvertNext
    const char *kCustomTableConvertNext =
        "status\tkey\tcommand\n"
        "Conversion\tSpace\tConvertNext\n";
    config.set_custom_keymap_table(kCustomTableConvertNext);

    KeyMapManager *keymap =
        KeyMapFactory::GetKeyMapManager(config::Config::CUSTOM);
    keymap->ReloadConfig(config);
    ConversionState::Commands key_command;
    keymap->GetCommandConversion(key, &key_command);
    EXPECT_EQ(ConversionState::CONVERT_NEXT, key_command);
  }

  // ConvertPrev
  const char *kCustomTableConvertPrev =
      "status\tkey\tcommand\n"
      "Conversion\tSpace\tConvertPrev\n";
  config.set_custom_keymap_table(kCustomTableConvertPrev);

  KeyMapManager *keymap =
      KeyMapFactory::GetKeyMapManager(config::Config::CUSTOM);
  keymap->ReloadConfig(config);
  ConversionState::Commands key_command;
  keymap->GetCommandConversion(key, &key_command);
  EXPECT_EQ(ConversionState::CONVERT_PREV, key_command);
}

}  // namespace keymap
}  // namespace mozc
