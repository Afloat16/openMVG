// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2012, 2013 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/feature.hpp"
#include "openMVG/features/descriptor.hpp"

#include "testing/testing.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

using namespace openMVG;
using namespace openMVG::features;

// Define a feature and a container of features
using Feature_T = SIOPointFeature;
using Feats_T = std::vector<Feature_T>;

// Define a descriptor and a container of descriptors
static const int DESC_LENGTH = 128;
using Desc_T = Descriptor<float, DESC_LENGTH>;
using Descs_T = std::vector<Desc_T, Eigen::aligned_allocator<Desc_T>>;

//--
//-- Features interface test
//--

static const int CARD = 12;

TEST(featureIO, NON_EXISTING_FILE) {

  // Try to read a non-existing feature file
  Feats_T vec_feats;
  EXPECT_FALSE(loadFeatsFromFile("x.feat", vec_feats));

  // Try to read a non-existing descriptor file
  Descs_T vec_descs;
  EXPECT_FALSE(loadDescsFromFile("x.desc", vec_descs));
  EXPECT_FALSE(loadDescsFromBinFile("x.desc", vec_descs));
}

TEST(featureIO, ASCII) {
  Feats_T vec_feats;
  for (int i = 0; i < CARD; ++i)
  {
    vec_feats.push_back(Feature_T(i, i*2, i*3, i*4));
  }

  //Save them to a file
  EXPECT_TRUE(saveFeatsToFile("tempFeats.feat", vec_feats));

  //Read the saved data and compare to input (to check write/read IO)
  Feats_T vec_feats_read;
  EXPECT_TRUE(loadFeatsFromFile("tempFeats.feat", vec_feats_read));
  EXPECT_EQ(CARD, vec_feats_read.size());

  for (int i = 0; i < CARD; ++i)
  {
    EXPECT_EQ(vec_feats[i], vec_feats_read[i]);
    EXPECT_EQ(vec_feats[i].coords(), vec_feats_read[i].coords());
    EXPECT_EQ(vec_feats[i].scale(), vec_feats_read[i].scale());
    EXPECT_EQ(vec_feats[i].orientation(), vec_feats_read[i].orientation());
  }
}

//--
//-- Descriptors interface test
//--
TEST(descriptorIO, ASCII) {
  // Create an input series of descriptor
  Descs_T vec_descs;
  for (int i = 0; i < CARD; ++i)
  {
    Desc_T desc;
    for (int j = 0; j < DESC_LENGTH; ++j)
      desc[j] = i*DESC_LENGTH+j;
    vec_descs.emplace_back(desc);
  }

  //Save them to a file
  EXPECT_TRUE(saveDescsToFile("tempDescs.desc", vec_descs));

  //Read the saved data and compare to input (to check write/read IO)
  Descs_T vec_descs_read;
  EXPECT_TRUE(loadDescsFromFile("tempDescs.desc", vec_descs_read));
  EXPECT_EQ(CARD, vec_descs_read.size());

  for (int i = 0; i < CARD; ++i)
  {
    for (int j = 0; j < DESC_LENGTH; ++j)
      EXPECT_EQ(vec_descs[i][j], vec_descs_read[i][j]);
  }
}

//Test binary export of descriptor
TEST(descriptorIO, BINARY) {
  // Create an input series of descriptor
  Descs_T vec_descs;
  for (int i = 0; i < CARD; ++i)
  {
    Desc_T desc;
    for (int j = 0; j < DESC_LENGTH; ++j)
      desc[j] = i*DESC_LENGTH+j;
    vec_descs.emplace_back(desc);
  }

  //Save them to a file
  EXPECT_TRUE(saveDescsToBinFile("tempDescsBin.desc", vec_descs));

  //Read the saved data and compare to input (to check write/read IO)
  Descs_T vec_descs_read;
  EXPECT_TRUE(loadDescsFromBinFile("tempDescsBin.desc", vec_descs_read));
  EXPECT_EQ(CARD, vec_descs_read.size());

  for (int i = 0; i < CARD; ++i)
  {
    for (int j = 0; j < DESC_LENGTH; ++j)
      EXPECT_EQ(vec_descs[i][j], vec_descs_read[i][j]);
  }
}

TEST(descriptorIO, BINARY_TRUNCATED_HEADER) {
  const std::size_t count = 2;
  for (const std::size_t length : {std::size_t(0), sizeof(count) - 1})
  {
    {
      std::ofstream file("truncated_header.desc", std::ios::binary);
      file.write(reinterpret_cast<const char *>(&count), length);
    }
    Descs_T descriptors(1);
    EXPECT_FALSE(loadDescsFromBinFile("truncated_header.desc", descriptors));
    EXPECT_TRUE(descriptors.empty());
  }
}

TEST(descriptorIO, BINARY_TRUNCATED_PAYLOAD) {
  const std::size_t count = 2;
  const std::vector<char> payload(2 * DESC_LENGTH * sizeof(float), 0);
  for (const std::size_t length : {std::size_t(0), payload.size() - 1})
  {
    {
      std::ofstream file("truncated_payload.desc", std::ios::binary);
      file.write(reinterpret_cast<const char *>(&count), sizeof(count));
      file.write(payload.data(), length);
    }
    Descs_T descriptors(1);
    EXPECT_FALSE(loadDescsFromBinFile("truncated_payload.desc", descriptors));
    EXPECT_TRUE(descriptors.empty());
  }
}

TEST(descriptorIO, BINARY_COUNT_EXCEEDS_PAYLOAD) {
  const std::size_t count = std::numeric_limits<std::size_t>::max();
  {
    std::ofstream file("invalid_count.desc", std::ios::binary);
    file.write(reinterpret_cast<const char *>(&count), sizeof(count));
  }
  Descs_T descriptors;
  bool threw_exception = false;
  bool loaded = false;
  try
  {
    loaded = loadDescsFromBinFile("invalid_count.desc", descriptors);
  }
  catch (const std::exception &)
  {
    threw_exception = true;
  }
  EXPECT_FALSE(threw_exception);
  EXPECT_FALSE(loaded);
  EXPECT_TRUE(descriptors.empty());
}

TEST(descriptorIO, BINARY_EMPTY_COLLECTION) {
  const Descs_T empty;
  EXPECT_TRUE(saveDescsToBinFile("empty_collection.desc", empty));
  Descs_T descriptors(1);
  EXPECT_TRUE(loadDescsFromBinFile("empty_collection.desc", descriptors));
  EXPECT_TRUE(descriptors.empty());
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */
