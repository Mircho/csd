#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "sensor.h"

void vo_1(observable_value_t *this) 
{
  printf("Value observer (1) %.3f\n", this->value.value);
}

void vo_2(observable_value_t *this)
{
  printf("Value observer (2) %f\n", this->value.value);
}

void test_generic(void) {
  OBSERVABLE_VALUE(pressure, 3.14);

  ADD_OBSERVER(pressure, vo_1);
  ADD_OBSERVER(pressure, vo_2);

  // must print two notifications
  NOTIFY_OBSERVERS(pressure);
  REMOVE_OBSERVER(pressure, vo_1);
  // must print one notification
  NOTIFY_OBSERVERS(pressure);

  FILTER_CLAMP(clamp_pressure_adc, 500, 700);
  FILTER_OFFSET(offset_pressure, -30);
  FILTER_SKIP(skip_values, 3);

  ADD_FILTER(pressure, skip_values);
  ADD_FILTER(pressure, clamp_pressure_adc);
  ADD_FILTER(pressure, offset_pressure);

  pressure.process(&pressure, 200);
  pressure.process(&pressure, 400);
  pressure.process(&pressure, 600);
  pressure.process(&pressure, 590);
  pressure.process(&pressure, 720);
  pressure.process(&pressure, 620);

  // CLEANUP_OBSERVERS(pressure);
}

void test_exp_m_avg(void) {
  double test_values[] = {
    500,
    510,
    520,
    530,
    540,
    550,
    560,
    570,
    580,
  };

  OBSERVABLE_VALUE(pressure_ema, 0);
  ADD_OBSERVER(pressure_ema, vo_1);

  FILTER_EXP_MOVING_AVERAGE(ema_pressure_filter, 0.8, true);
  ADD_FILTER(pressure_ema, ema_pressure_filter);

  for(size_t cnt = 0; cnt < (sizeof(test_values)/sizeof(test_values[0])); cnt++) {
    pressure_ema.process(&pressure_ema, test_values[cnt]);
  }

  printf("Result for EMA: %.4f\n", pressure_ema.value.value);
  const double expected = 577.5;
  assert( fabs( pressure_ema.value.value - expected ) < 0.05 );
  // CLEANUP_OBSERVERS(pressure_ema);
}

void test_avg(void) {
  double test_values[] = {
    1,
    2,
    3,
    10,
    20,
    30,
    100,
    200,
    300,
    1000,
    2000,
    3000,
  };

  OBSERVABLE_VALUE(pressure_a, 0);
  ADD_OBSERVER(pressure_a, vo_1);

  FILTER_AVERAGE(a_pressure_filter, 3, false);
  ADD_FILTER(pressure_a, a_pressure_filter);

  for(size_t cnt = 0; cnt < (sizeof(test_values)/sizeof(test_values[0])); cnt++) {
    PROCESS_NEW_VALUE(pressure_a, test_values[cnt]);
  }

  printf("Result for A: %.4f\n", pressure_a.value.value);
  const double expected = 2000.0;
  assert(fabs(pressure_a.value.value - expected) < 0.05);

  // CLEANUP_OBSERVERS(pressure_a);
}

void test_linear_fit(void) {
  OBSERVABLE_VALUE(pressure_lf, 0);
  ADD_OBSERVER(pressure_lf, vo_1);

//  FILTER_LINEAR_FIT(a_pressure_filter, 1,1,10,50);
  FILTER_LINEAR_FIT(a_pressure_filter, 500,0,685,100);
  printf("Slope: %f, Intercept: %f\n", a_pressure_filter.slope_, a_pressure_filter.intercept_);
  ADD_FILTER(pressure_lf, a_pressure_filter);

  FILTER_CLAMP(a_pressure_filter_c, 0, 100);
  ADD_FILTER(pressure_lf, a_pressure_filter_c);

  PROCESS_NEW_VALUE(pressure_lf, 550);

  printf("Result for A: %.4f\n", pressure_lf.value.value);
  const double expected_550 = 27.027027027027;
  assert(fabs(pressure_lf.value.value - expected_550) < 0.05);

  PROCESS_NEW_VALUE(pressure_lf, 617);

  printf("Result for A: %.4f\n", pressure_lf.value.value);
  const double expected_617 = 63.2432432432432;
  assert(fabs(pressure_lf.value.value - expected_617) < 0.05);

  // expected 27.778
  // CLEANUP_OBSERVERS(pressure_lf);
}

int main(void)
{
  test_generic();

  test_exp_m_avg();

  test_avg();

  test_linear_fit();

  return 0;
}