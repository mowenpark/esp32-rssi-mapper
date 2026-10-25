#include <string>

#include "check.h"
#include "signal_level.h"

void test_signal_level() {
  CHECK(ratingFor(-30) == Rating::Excellent);
  CHECK(ratingFor(-50) == Rating::Excellent);
  CHECK(ratingFor(-51) == Rating::Good);
  CHECK(ratingFor(-60) == Rating::Good);
  CHECK(ratingFor(-61) == Rating::Fair);
  CHECK(ratingFor(-70) == Rating::Fair);
  CHECK(ratingFor(-71) == Rating::Poor);
  CHECK(ratingFor(-95) == Rating::Poor);
  CHECK_EQ(std::string(ratingName(Rating::Good)), std::string("GOOD"));
  CHECK_EQ(ratingColor(Rating::Fair), Colors::Fair);
  CHECK_EQ(barPercent(-30), 100);
  CHECK_EQ(barPercent(-25), 100);  // clamp high
  CHECK_EQ(barPercent(-52), 63);
  CHECK_EQ(barPercent(-78), 20);
  CHECK_EQ(barPercent(-90), 4);    // clamp low
  CHECK_EQ(barPercent(-95), 4);
  CHECK_EQ(nodeCountText(0), std::string("0 nodes heard"));
  CHECK_EQ(nodeCountText(1), std::string("1 node heard"));
  CHECK_EQ(nodeCountText(3), std::string("3 nodes heard"));
}
