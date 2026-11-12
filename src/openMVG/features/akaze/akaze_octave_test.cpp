// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/akaze/AKAZE.hpp"
#include "openMVG/image/image_resampling.hpp"
#include "testing/testing.h"

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::image;

TEST(AKAZEOctave, HalfSampleUsesPixelBlockCenters)
{
  Image<float> input(6, 4);
  for (int y = 0; y < input.Height(); ++y)
    for (int x = 0; x < input.Width(); ++x)
      input(y, x) = 10.f * y + x;

  Image<float> output;
  ImageHalfSample(input, output);
  EXPECT_EQ(3, output.Width());
  EXPECT_EQ(2, output.Height());
  for (int y = 0; y < output.Height(); ++y)
    for (int x = 0; x < output.Width(); ++x)
      EXPECT_NEAR(10.f * (2.f * y + .5f) + 2.f * x + .5f,
                  output(y, x), 1e-6);

  // Averaging each 2x2 checkerboard block must retain its mean intensity.
  for (int y = 0; y < input.Height(); ++y)
    for (int x = 0; x < input.Width(); ++x)
      input(y, x) = (x + y) % 2;
  ImageHalfSample(input, output);
  for (int y = 0; y < output.Height(); ++y)
    for (int x = 0; x < output.Width(); ++x)
      EXPECT_NEAR(.5f, output(y, x), 1e-6);
}

TEST(AKAZEOctave, RefinementAppliesPixelCenterOffsetOnce)
{
  const AKAZE detector(Image<unsigned char>{}, AKAZE::Params());
  Image<float> response(9, 9);
  for (int y = 0; y < response.Height(); ++y)
    for (int x = 0; x < response.Width(); ++x)
      response(y, x) = 100.f - (x - 4.25f) * (x - 4.25f)
                            - (y - 3.75f) * (y - 3.75f);

  for (unsigned char octave = 0; octave < 4; ++octave)
  {
    const float ratio = 1 << octave;
    const float offset = .5f * (ratio - 1.f);
    AKAZEKeypoint keypoint;
    keypoint.octave = octave;
    keypoint.x = 4.f * ratio + offset;
    keypoint.y = 4.f * ratio + offset;
    EXPECT_TRUE(detector.Do_Subpixel_Refinement(keypoint, response));
    EXPECT_NEAR(4.25f * ratio + offset, keypoint.x, 1e-6);
    EXPECT_NEAR(3.75f * ratio + offset, keypoint.y, 1e-6);
  }
}

int main()
{
  TestResult result;
  return TestRegistry::runAllTests(result);
}
