// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/sift/hierarchical_gaussian_scale_space.hpp"
#include "openMVG/image/image_filtering.hpp"
#include "testing/testing.h"

using namespace openMVG;
using namespace openMVG::image;

TEST(ImageGaussian, FractionalScalesPreserveReflectionSymmetry)
{
  const int center = 32;
  Image<float> input(65, 65);
  input(center, center) = 1.f;

  for (const double sigma : {0.5, 1.2, 1.6, 2.5, 5.0})
  {
    Image<float> output;
    ImageGaussianFilter(input, sigma, output);
    EXPECT_NEAR(1.f, output.sum(), 1e-6);
    for (int offset = 1; offset <= 20; ++offset)
    {
      EXPECT_NEAR(output(center, center - offset),
                  output(center, center + offset), 1e-6);
      EXPECT_NEAR(output(center - offset, center),
                  output(center + offset, center), 1e-6);
    }
  }
}

TEST(ImageGaussian, SiftScaleSpacePreservesImpulseLocation)
{
  Image<float> input(128, 128);
  input(64, 64) = 1.f;
  features::HierarchicalGaussianScaleSpace pyramid(1, 3);
  pyramid.SetImage(input);
  features::Octave octave;
  EXPECT_TRUE(pyramid.NextOctave(octave));
  EXPECT_EQ(3, octave.slices.size());
  for (const auto & slice : octave.slices)
  {
    double total = 0.0, moment_x = 0.0, moment_y = 0.0;
    for (int y = 0; y < slice.Height(); ++y)
      for (int x = 0; x < slice.Width(); ++x)
      {
        total += slice(y, x);
        moment_x += x * static_cast<double>(slice(y, x));
        moment_y += y * static_cast<double>(slice(y, x));
      }
    EXPECT_NEAR(64.0, moment_x / total, 1e-5);
    EXPECT_NEAR(64.0, moment_y / total, 1e-5);
  }
}

int main()
{
  TestResult result;
  return TestRegistry::runAllTests(result);
}
