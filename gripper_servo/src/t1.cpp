#include <unistd.h>

#include <cstdio>
#include <iostream>

class simplefs {
 public:
  std::string fs_path;

  simplefs(std::string desire_path) { fs_path = desire_path; }

  bool set(std::string info, std::string value) {
    std::cout << " info: " << info << "  value:" << value << std::endl;
    std::string thefileName = fs_path + "/" + info;
    std::FILE *thefile = fopen(thefileName.c_str(), "w");
    if (!thefile) return false;
    // no buffer
    std::setbuf(thefile, 0);
    fwrite(value.c_str(), value.length(), 1, thefile);
    fclose(thefile);
    usleep(10000);  // add a small delay

    return true;
  }

  inline bool set_unexport(int servoId) {
    return set("unexport", std::to_string(servoId));
  }

  inline bool set_export(int servoId) {
    return set("export", std::to_string(servoId));
  }

  inline bool set_enable(int servoId, bool flag) {
    return set("pwm" + std::to_string(servoId) + "/enable", flag ? "1" : "0");
  }

  inline bool set_period(int servoId, int period) {
    return set("pwm" + std::to_string(servoId) + "/period",
               std::to_string(period));
  }

  inline bool set_duty_cycle(int servoId, int duty_cycle) {
    return set("pwm" + std::to_string(servoId) + "/duty_cycle",
               std::to_string(duty_cycle));
  }
};

int main(int argc, char **argv) {
  const int period_in_ns = 20000000;
  const int servo_center = 1500000;
  const int servo_90P = 500000;
  const int servo_90N = 2500000;
  const int cycle_count = 4;
  const int servo_count = 1;
  const int cycles[cycle_count] = {servo_90P, servo_center, servo_90N,
                                   servo_center};

  simplefs servos = simplefs("/sys/class/pwm/pwmchip0");

  // unexport all
  for (int loop = 0; loop < servo_count; loop++) servos.set_unexport(loop);

  // export all
  for (int loop = 0; loop < servo_count; loop++) servos.set_export(loop);

  // wait a little
  usleep(100000);

  for (int loop = 0; loop < servo_count; loop++) {
    servos.set_period(loop, period_in_ns);
    servos.set_duty_cycle(loop, servo_center);
    servos.set_enable(loop, true);
  }

  // ok let's do something
  useconds_t delay = 50'000;
  while (true) {
    // All together do something
    for (int value = servo_90P; value < servo_90N; value = value + (servo_90N - servo_90P) / 100) {
      servos.set_duty_cycle(0, value);
      usleep(delay);
    }
    for (int value = servo_90N; value > servo_90P; value = value - (servo_90N - servo_90P) / 100) {
      servos.set_duty_cycle(0, value);
      usleep(delay);
    }
  }

  // disable & unexport
  for (int servo = 0; servo < servo_count; servo++) {
    servos.set_enable(servo, false);
    //         servos.set_unexport(servo);
  }

  return 0;
}