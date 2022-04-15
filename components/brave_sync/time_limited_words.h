/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SYNC_TIME_LIMITED_WORDS_H_
#define BRAVE_COMPONENTS_BRAVE_SYNC_TIME_LIMITED_WORDS_H_

#include <map>
#include <string>

#include "base/gtest_prod_util.h"
#include "base/time/time.h"

namespace brave_sync {

enum class WordsValidationResult {
  kValid = 0,
  kNotValidPureWords = 1,
  kVersionDeprecated = 2,
  kExpired = 3,
  kValidForTooLong = 4,
  kWrongWordsNumber = 5,
};

FORWARD_DECLARE_TEST(TimeLimitedWordsTest, GenerateForDate);
FORWARD_DECLARE_TEST(TimeLimitedWordsTest, GetIndexByWord);
FORWARD_DECLARE_TEST(TimeLimitedWordsTest, GetRoundedDaysDiff);
FORWARD_DECLARE_TEST(TimeLimitedWordsTest, GetWordByIndex);
FORWARD_DECLARE_TEST(TimeLimitedWordsTest, Validate);

class TimeLimitedWords {
 public:
  static std::string GenerateForNow(const std::string& pure_words);

  static WordsValidationResult Validate(const std::string& time_limited_words,
                                        std::string* pure_words);

  static base::Time GetWordsV1SunsetDay();
  static base::Time GetWordsV2Epoch();

 private:
  FRIEND_TEST_ALL_PREFIXES(TimeLimitedWordsTest, GenerateForDate);
  FRIEND_TEST_ALL_PREFIXES(TimeLimitedWordsTest, GetIndexByWord);
  FRIEND_TEST_ALL_PREFIXES(TimeLimitedWordsTest, GetRoundedDaysDiff);
  FRIEND_TEST_ALL_PREFIXES(TimeLimitedWordsTest, GetWordByIndex);
  FRIEND_TEST_ALL_PREFIXES(TimeLimitedWordsTest, Validate);

  static std::string GenerateForDate(const std::string& pure_words,
                                     const base::Time& not_after);
  static int GetRoundedDaysDiff(const base::Time& time1,
                                const base::Time& time2);

  static std::string GetWordByIndex(size_t index);
  static int GetIndexByWord(const std::string& word);

  static base::Time words_v1_sunset_day_;
  static base::Time words_v2_epoch_;
};

}  // namespace brave_sync

#endif  // BRAVE_COMPONENTS_BRAVE_SYNC_TIME_LIMITED_WORDS_H_
