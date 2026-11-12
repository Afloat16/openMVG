// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/akaze/AKAZE.hpp"
#include "testing/testing.h"

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::image;

TEST(AKAZEContrast, ConstantImagesKeepFiniteScaleSpace)
{
  AKAZE::Params options;
  options.iNbOctave = 2;
  options.iNbSlicePerOctave = 2;

  for (const unsigned char intensity : {0, 128, 255})
  {
    const Image<unsigned char> image(64, 64, true, intensity);
    AKAZE detector(image, options);
    detector.Compute_AKAZEScaleSpace();
    EXPECT_EQ(4, detector.getSlices().size());
    for (const auto & slice : detector.getSlices())
    {
      EXPECT_TRUE(slice.cur.allFinite());
      EXPECT_TRUE(slice.Lx.allFinite());
      EXPECT_TRUE(slice.Ly.allFinite());
      EXPECT_TRUE(slice.Lhess.allFinite());
      EXPECT_NEAR(intensity / 255.f, slice.cur.minCoeff(), 1e-6);
      EXPECT_NEAR(intensity / 255.f, slice.cur.maxCoeff(), 1e-6);
    }
    std::vector<AKAZEKeypoint> keypoints;
    detector.Feature_Detection(keypoints);
    EXPECT_TRUE(keypoints.empty());
  }
}

int main()
{
  TestResult result;
  return TestRegistry::runAllTests(result);
}
