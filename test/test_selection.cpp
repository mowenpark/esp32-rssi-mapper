#include <string>
#include <vector>

#include "check.h"
#include "selection.h"

void test_selection() {
  std::vector<ScanResult> r = {
      {"Neighbor", "00:00:00:00:00:01", -40},
      {"Home", "AA:BB:CC:DD:3F:A1", -61},
      {"Home", "AA:BB:CC:DD:12:34", -55},
      {"Home", "AA:BB:CC:DD:56:78", -55},
  };
  Pick p = pickStrongest(r, "Home");
  CHECK(p.found);
  CHECK_EQ(p.bssid, std::string("AA:BB:CC:DD:12:34"));  // tie -> earlier
  CHECK_EQ(p.rssi, -55);
  CHECK_EQ(p.nodeCount, 3);
  CHECK(!pickStrongest(r, "home").found);  // case-sensitive SSID
  CHECK(!pickStrongest({}, "Home").found);
  CHECK_EQ(pickStrongest({}, "Home").nodeCount, 0);

  NodeName names[] = {{"3f:a1", "Living Room"}};
  CHECK_EQ(nodeLabel("AA:BB:CC:DD:3F:A1", names, 1), std::string("Living Room"));
  CHECK_EQ(nodeLabel("aa:bb:cc:dd:12:34", names, 1), std::string("Node 12:34"));
  CHECK_EQ(nodeLabel("AA:BB:CC:DD:12:34", nullptr, 0), std::string("Node 12:34"));

  Smoother s;
  CHECK_EQ(s.add("A", -50), -50);
  CHECK_EQ(s.add("A", -54), -52);
  CHECK_EQ(s.add("A", -58), -54);
  CHECK_EQ(s.add("A", -62), -58);  // window of 3: -54, -58, -62
  CHECK_EQ(s.add("B", -70), -70);  // node change resets
  CHECK_EQ(s.add("A", -40), -40);  // flip back: no mixing with old A or B
  CHECK_EQ(s.add("A", -41), -41);  // mean -40.5 -> lround -> -41
  s.reset();
  CHECK_EQ(s.add("A", -60), -60);
}
