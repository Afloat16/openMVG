// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/akaze/AKAZE.hpp"
#include "testing/testing.h"

#include <cmath>

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::image;

TEST(AKAZEScaleSpace, FewerSlicesThanOctaves)
{
  Image<unsigned char> image(128, 128);
  for (int y = 0; y < image.Height(); ++y)
    for (int x = 0; x < image.Width(); ++x)
    {
      const float dx = x - 65.f;
      const float dy = y - 61.f;
      image(y, x) = 20 + 200 * std::exp(-(dx * dx + dy * dy) / 32.f);
    }

  AKAZE::Params options;
  options.iNbOctave = 2;
  options.iNbSlicePerOctave = 1;
  options.fThreshold = 1e-6f;
  AKAZE detector(image, options);
  detector.Compute_AKAZEScaleSpace();
  EXPECT_EQ(2, detector.getSlices().size());

  std::vector<AKAZEKeypoint> keypoints;
  detector.Feature_Detection(keypoints);
  EXPECT_FALSE(keypoints.empty());
  bool found_second_octave = false;
  for (const auto & point : keypoints)
  {
    EXPECT_TRUE(point.class_id < detector.getSlices().size());
    EXPECT_EQ(point.octave, point.class_id);
    found_second_octave |= point.octave == 1;
  }
  EXPECT_TRUE(found_second_octave);
}

int main()
{
  TestResult result;
  return TestRegistry::runAllTests(result);
}
