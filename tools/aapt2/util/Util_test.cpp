/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util/Util.h"

#include <string>

#include "test/Test.h"

using android::StringPiece;

namespace aapt {

TEST(UtilTest, TrimOnlyWhitespace) {
  const std::string full = "\n        ";

  StringPiece trimmed = util::TrimWhitespace(full);
  EXPECT_TRUE(trimmed.empty());
  EXPECT_EQ(0u, trimmed.size());
}

TEST(UtilTest, StringEndsWith) {
  EXPECT_TRUE(util::EndsWith("hello.xml", ".xml"));
}

TEST(UtilTest, StringStartsWith) {
  EXPECT_TRUE(util::StartsWith("hello.xml", "he"));
}

TEST(UtilTest, StringBuilderSplitEscapeSequence) {
  EXPECT_EQ(StringPiece("this is a new\nline."), util::StringBuilder()
                                                     .Append("this is a new\\")
                                                     .Append("nline.")
                                                     .ToString());
}

TEST(UtilTest, StringBuilderWhitespaceRemoval) {
  EXPECT_EQ(StringPiece("hey guys this is so cool"),
            util::StringBuilder()
                .Append("    hey guys ")
                .Append(" this is so cool ")
                .ToString());

  EXPECT_EQ(StringPiece(" wow,  so many \t spaces. what?"),
            util::StringBuilder()
                .Append(" \" wow,  so many \t ")
                .Append("spaces. \"what? ")
                .ToString());

  EXPECT_EQ(StringPiece("where is the pie?"), util::StringBuilder()
                                                  .Append("  where \t ")
                                                  .Append(" \nis the "
                                                          " pie?")
                                                  .ToString());
}

TEST(UtilTest, StringBuilderEscaping) {
  EXPECT_EQ(StringPiece("hey guys\n this \t is so\\ cool"),
            util::StringBuilder()
                .Append("    hey guys\\n ")
                .Append(" this \\t is so\\\\ cool ")
                .ToString());

  EXPECT_EQ(StringPiece("@?#\\\'"),
            util::StringBuilder().Append("\\@\\?\\#\\\\\\'").ToString());
}

TEST(UtilTest, StringBuilderMisplacedQuote) {
  util::StringBuilder builder{};
  EXPECT_FALSE(builder.Append("they're coming!"));
}

TEST(UtilTest, StringBuilderUnicodeCodes) {
  EXPECT_EQ(std::string("\u00AF\u0AF0 woah"),
            util::StringBuilder().Append("\\u00AF\\u0AF0 woah").ToString());

  EXPECT_FALSE(util::StringBuilder().Append("\\u00 yo"));
}

TEST(UtilTest, TokenizeInput) {
  auto tokenizer = util::Tokenize(StringPiece("this| is|the|end"), '|');
  auto iter = tokenizer.begin();
  ASSERT_EQ(*iter, StringPiece("this"));
  ++iter;
  ASSERT_EQ(*iter, StringPiece(" is"));
  ++iter;
  ASSERT_EQ(*iter, StringPiece("the"));
  ++iter;
  ASSERT_EQ(*iter, StringPiece("end"));
  ++iter;
  ASSERT_EQ(tokenizer.end(), iter);
}

TEST(UtilTest, TokenizeEmptyString) {
  auto tokenizer = util::Tokenize(StringPiece(""), '|');
  auto iter = tokenizer.begin();
  ASSERT_NE(tokenizer.end(), iter);
  ASSERT_EQ(StringPiece(), *iter);
  ++iter;
  ASSERT_EQ(tokenizer.end(), iter);
}

TEST(UtilTest, TokenizeAtEnd) {
  auto tokenizer = util::Tokenize(StringPiece("one."), '.');
  auto iter = tokenizer.begin();
  ASSERT_EQ(*iter, StringPiece("one"));
  ++iter;
  ASSERT_NE(iter, tokenizer.end());
  ASSERT_EQ(*iter, StringPiece());
}

TEST(UtilTest, IsJavaClassName) {
  EXPECT_TRUE(util::IsJavaClassName("android.test.Class"));
  EXPECT_TRUE(util::IsJavaClassName("android.test.Class$Inner"));
  EXPECT_TRUE(util::IsJavaClassName("android_test.test.Class"));
  EXPECT_TRUE(util::IsJavaClassName("_android_.test._Class_"));
  EXPECT_FALSE(util::IsJavaClassName("android.test.$Inner"));
  EXPECT_FALSE(util::IsJavaClassName("android.test.Inner$"));
  EXPECT_FALSE(util::IsJavaClassName(".test.Class"));
  EXPECT_FALSE(util::IsJavaClassName("android"));
}

TEST(UtilTest, IsJavaPackageName) {
  EXPECT_TRUE(util::IsJavaPackageName("android"));
  EXPECT_TRUE(util::IsJavaPackageName("android.test"));
  EXPECT_TRUE(util::IsJavaPackageName("android.test_thing"));
  EXPECT_FALSE(util::IsJavaPackageName("_android"));
  EXPECT_FALSE(util::IsJavaPackageName("android_"));
  EXPECT_FALSE(util::IsJavaPackageName("android."));
  EXPECT_FALSE(util::IsJavaPackageName(".android"));
  EXPECT_FALSE(util::IsJavaPackageName("android._test"));
  EXPECT_FALSE(util::IsJavaPackageName(".."));
}

TEST(UtilTest, FullyQualifiedClassName) {
  Maybe<std::string> res = util::GetFullyQualifiedClassName("android", ".asdf");
  AAPT_ASSERT_TRUE(res);
  EXPECT_EQ(res.value(), "android.asdf");

  res = util::GetFullyQualifiedClassName("android", ".a.b");
  AAPT_ASSERT_TRUE(res);
  EXPECT_EQ(res.value(), "android.a.b");

  res = util::GetFullyQualifiedClassName("android", "a.b");
  AAPT_ASSERT_TRUE(res);
  EXPECT_EQ(res.value(), "a.b");

  res = util::GetFullyQualifiedClassName("", "a.b");
  AAPT_ASSERT_TRUE(res);
  EXPECT_EQ(res.value(), "a.b");

  res = util::GetFullyQualifiedClassName("android", "Class");
  AAPT_ASSERT_TRUE(res);
  EXPECT_EQ(res.value(), "android.Class");

  res = util::GetFullyQualifiedClassName("", "");
  AAPT_ASSERT_FALSE(res);

  res = util::GetFullyQualifiedClassName("android", "./Apple");
  AAPT_ASSERT_FALSE(res);
}

TEST(UtilTest, ExtractResourcePathComponents) {
  StringPiece prefix, entry, suffix;
  ASSERT_TRUE(util::ExtractResFilePathParts("res/xml-sw600dp/entry.xml",
                                            &prefix, &entry, &suffix));
  EXPECT_EQ(prefix, "res/xml-sw600dp/");
  EXPECT_EQ(entry, "entry");
  EXPECT_EQ(suffix, ".xml");

  ASSERT_TRUE(util::ExtractResFilePathParts("res/xml-sw600dp/entry.9.png",
                                            &prefix, &entry, &suffix));

  EXPECT_EQ(prefix, "res/xml-sw600dp/");
  EXPECT_EQ(entry, "entry");
  EXPECT_EQ(suffix, ".9.png");

  EXPECT_FALSE(util::ExtractResFilePathParts("AndroidManifest.xml", &prefix,
                                             &entry, &suffix));
  EXPECT_FALSE(
      util::ExtractResFilePathParts("res/.xml", &prefix, &entry, &suffix));

  ASSERT_TRUE(
      util::ExtractResFilePathParts("res//.", &prefix, &entry, &suffix));
  EXPECT_EQ(prefix, "res//");
  EXPECT_EQ(entry, "");
  EXPECT_EQ(suffix, ".");
}

TEST(UtilTest, VerifyJavaStringFormat) {
  ASSERT_TRUE(util::VerifyJavaStringFormat("%09.34f"));
  ASSERT_TRUE(util::VerifyJavaStringFormat("%9$.34f %8$"));
  ASSERT_TRUE(util::VerifyJavaStringFormat("%% %%"));
  ASSERT_FALSE(util::VerifyJavaStringFormat("%09$f %f"));
  ASSERT_FALSE(util::VerifyJavaStringFormat("%09f %08s"));
}

}  // namespace aapt
