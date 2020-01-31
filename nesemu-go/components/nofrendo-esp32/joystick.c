// #include "joystick.h"
//
// #include "driver/i2c.h"
// #include "driver/gpio.h"
// #include <driver/adc.h>
//
// #include <string.h>
//
//
// #define PCF8574_DIGITAL_ADDR 0x20
// #define PCF8574A_DIGITAL_ADDR 0x38
//
// #define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0
// #define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0
//
// #define WRITE_BIT  I2C_MASTER_WRITE /*!< I2C master write */
// #define READ_BIT I2C_MASTER_READ /*!< I2C master read */
// #define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
// #define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */
// #define ACK_VAL    0x0         /*!< I2C ack value */
// #define NACK_VAL 0x1 /*!< I2C nack value */
//
// #define SDA 15
// #define SCL 4
// #define JOYSTICK_ENABLED 1
//
// #define SHOW3_JOYSTICK 0
// #define ODROID_JOYSTICK 1
//
// uint8_t JoystickI2cAddress;
// uint8_t JoystickEnabled;
//
//
// #if ODROID_JOYSTICK
//
//
//
// void JoystickInit()
// {
//
// 	gpio_set_direction(JOY_SELECT, GPIO_MODE_INPUT);
// 	gpio_set_pull_mode(JOY_SELECT, GPIO_PULLUP_ONLY);
//
// 	gpio_set_direction(JOY_START, GPIO_MODE_INPUT);
//
// 	gpio_set_direction(JOY_A, GPIO_MODE_INPUT);
// 	gpio_set_pull_mode(JOY_A, GPIO_PULLUP_ONLY);
//
//     gpio_set_direction(JOY_B, GPIO_MODE_INPUT);
// 	gpio_set_pull_mode(JOY_B, GPIO_PULLUP_ONLY);
//
// 	adc1_config_width(ADC_WIDTH_12Bit);
//     adc1_config_channel_atten(JOY_X, ADC_ATTEN_11db);	// JOY-X
// 	adc1_config_channel_atten(JOY_Y, ADC_ATTEN_11db);	// JOY-Y
//
// 	gpio_set_direction(BTN_MENU, GPIO_MODE_INPUT);
// 	gpio_set_pull_mode(BTN_MENU, GPIO_PULLUP_ONLY);
//
// 	gpio_set_direction(BTN_VOLUME, GPIO_MODE_INPUT);
// 	//gpio_set_pull_mode(BTN_VOLUME, GPIO_PULLUP_ONLY);
//
//   printf("JoystickInit: done.\n");
//
// }
//
// JoystickState JoystickRead()
// {
// 	JoystickState state;
//
//
// 	int joyX = adc1_get_raw(ADC1_CHANNEL_6);
// 	int joyY = adc1_get_raw(ADC1_CHANNEL_7);
//
// 	if (joyX > 2048 + 1024)
// 	{
// 		state.Left = 1;
// 		state.Right = 0;
// 	}
// 	else if (joyX > 1024)
// 	{
// 		state.Left = 0;
// 		state.Right = 1;
// 	}
// 	else
// 	{
// 		state.Left = 0;
// 		state.Right = 0;
// 	}
//
// 	if (joyY > 2048 + 1024)
// 	{
// 		state.Up = 1;
// 		state.Down = 0;
// 	}
// 	else if (joyY > 1024)
// 	{
// 		state.Up = 0;
// 		state.Down = 1;
// 	}
// 	else
// 	{
// 		state.Up = 0;
// 		state.Down = 0;
// 	}
//
// 	state.Select = !(gpio_get_level(JOY_SELECT));
// 	state.Start = !(gpio_get_level(JOY_START));
//
//     state.A = !(gpio_get_level(JOY_A));
//     state.B = !(gpio_get_level(JOY_B));
//
// 	state.Menu = !(gpio_get_level(BTN_MENU));
// 	state.Volume = !(gpio_get_level(BTN_VOLUME));
//
// 	return state;
// }
//
// #elif SHOW3_JOYSTICK
//
// JoystickState JoystickRead()
// {
// 	JoystickState state;
//
// #if 0
// 	// select
// 	if (gpio_get_level(GPIO_NUM_16))
// 		result |= (1 << 0);
//
// 	// start
// 	if (gpio_get_level(GPIO_NUM_17))
// 		result |= (1 << 3);
// #endif
//
// 	const int DEAD_ZONE = 768;
//
// 	int joyX = adc1_get_raw(ADC1_CHANNEL_6);
// 	int joyY = adc1_get_raw(ADC1_CHANNEL_7);
//
//     state.Right = (joyX > (2048 + DEAD_ZONE));
//     state.Left = (joyX < (2048 - DEAD_ZONE));
//     state.Down = (joyY < (2048 - DEAD_ZONE));
//     state.Up = (joyY > (2048 + DEAD_ZONE));
//
// 	state.Select = !(gpio_get_level(GPIO_NUM_13));
// 	state.Start = !(gpio_get_level(GPIO_NUM_0));
//
//     //state.Select = !(gpio_get_level(GPIO_NUM_27));
//     //state.Start = !(gpio_get_level(GPIO_NUM_25));
//
//     state.A = !(gpio_get_level(GPIO_NUM_32));
//     state.B = !(gpio_get_level(GPIO_NUM_33));
//
// 	return state;
// }
//
// #else
//
// JoystickState JoystickRead()
// {
//   JoystickState state;
//
//   if (JoystickEnabled)
//   {
//     i2c_cmd_handle_t cmd;
//     int ret;
//     uint8_t buttons;
//
//     // set outputs high
//     cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, JoystickI2cAddress << 1 | WRITE_BIT, ACK_CHECK_EN);
//     i2c_master_write_byte(cmd, 0xff, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//
//     //printf("JoystickRead: write - ret=%d\n", ret);
//
//     // read the buttons
//     cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, JoystickI2cAddress << 1 | READ_BIT, ACK_CHECK_EN);
//     i2c_master_read_byte(cmd, &buttons, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//
//     //printf("JoystickRead: read - ret=%d, buttons=0x%x\n", ret, buttons);
//
//     // set outputs low
//     cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, JoystickI2cAddress << 1 | WRITE_BIT, ACK_CHECK_EN);
//     i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//
//     //printf("JoystickRead: write - ret=%d\n", ret);
//
//
//     // if (ret != ESP_OK)
//     //   JoystickInit();
//
//     state.Up = !(buttons & (1 << 0));
//     state.Right = !(buttons & (1 << 1));
//     state.Down = !(buttons & (1 << 2));
//     state.Left = !(buttons & (1 << 3));
//     state.Select = !(buttons & (1 << 4));
//     state.Start = !(buttons & (1 << 5));
//     state.B = !(buttons & (1 << 6));
//     state.A = !(buttons & (1 << 7));
//   }
//   else
//   {
// 	   memset(&state, 0, sizeof(state));
//   }
//
// 	return state;
// }
//
//
// void resetI2C()
// {
// 	// https://github.com/espressif/esp-idf/issues/922
//
// 	/*
// 	If the data line (SDA) is stuck LOW, the master should send nine clock pulses. The device
// 	that held the bus LOW should release it sometime within those nine clocks.
// 	*/
//
// 	gpio_config_t scl_conf = {0};
// 	scl_conf.intr_type = GPIO_PIN_INTR_DISABLE;
// 	scl_conf.mode = GPIO_MODE_OUTPUT;
// 	scl_conf.pin_bit_mask = (1 << SCL);
// 	scl_conf.pull_down_en = 0;
// 	scl_conf.pull_up_en = 1;
//
// 	gpio_config(&scl_conf);
//
//
// 	gpio_config_t sda_conf = {0};
// 	sda_conf.intr_type = GPIO_PIN_INTR_DISABLE;
// 	sda_conf.mode = GPIO_MODE_INPUT;
// 	sda_conf.pin_bit_mask = (1 << SDA);
// 	sda_conf.pull_down_en = 0;
// 	sda_conf.pull_up_en = 1;
//
// 	gpio_config(&sda_conf);
//
// 	if (!gpio_get_level(SDA))
// 	{
// 		printf("Recovering i2c bus.\n");
// 		for (int i = 0; i < 9; ++i)
// 		{
// 			gpio_set_level(SCL, 1);
// 			vTaskDelay(5 / portTICK_PERIOD_MS);
//
// 			gpio_set_level(SCL, 0);
// 			vTaskDelay(5 / portTICK_PERIOD_MS);
// 		}
// 	}
// 	else
// 	{
// 		printf("Skipping i2c bus recovery.\n");
// 	}
// }
//
// void ProbeJoystick()
// {
//   uint8_t address[2] = {PCF8574_DIGITAL_ADDR, PCF8574A_DIGITAL_ADDR};
//   uint8_t found = 0;
//   i2c_cmd_handle_t cmd;
//   int ret;
//   uint8_t buttons;
//
//   for (int i = 0; i < 2; ++i)
//   {
//     JoystickI2cAddress = address[i];
//
//     // read the buttons
//     cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, JoystickI2cAddress << 1 | READ_BIT, ACK_CHECK_EN);
//     i2c_master_read_byte(cmd, &buttons, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//
//     if (ret == ESP_OK)
//     {
//       found = 1;
//
//       printf("ProbeJoystick: found at I2C 0x%x\n", JoystickI2cAddress);
//       break;
//     }
//   }
//
//   JoystickEnabled = found;
//
//   if (!found)
//   {
//     printf("ProbeJoystick: not found.\n");
//   }
// }
//
// #endif
//
//
// // void JoystickInit()
// // {
// //
// // #if SHOW3_JOYSTICK
// //
// // 	gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);	// Select (left - bottom)
// // 	gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);	// Start (right)
// // 	//GPIO_NUM_39 (left - top)
// //
// // 	//gpio_set_direction(GPIO_NUM_27, GPIO_MODE_INPUT);	// Select
// // 	//gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);	// Start
// //
// // 	gpio_set_direction(32, GPIO_MODE_INPUT);	// A
// //     gpio_set_direction(33, GPIO_MODE_INPUT);	// B
// //
// // 	adc1_config_width(ADC_WIDTH_12Bit);
// //     adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);	// JOY-X
// // 	adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_11db);	// JOY-Y
// //
// // #else
// //
// // 	resetI2C();
// //
// // 	int i2c_master_port = I2C_NUM_1;
// // 	i2c_config_t conf;
// // 	conf.mode = I2C_MODE_MASTER;
// // 	conf.sda_io_num = SDA;
// // 	conf.sda_pullup_en = GPIO_PULLUP_DISABLE /*GPIO_PULLUP_ENABLE*/;
// // 	conf.scl_io_num = SCL;
// // 	conf.scl_pullup_en = GPIO_PULLUP_DISABLE /*GPIO_PULLUP_ENABLE*/;
// // 	conf.master.clk_speed = 100000; // Hz
// // 	i2c_param_config(i2c_master_port, &conf);
// // 	i2c_driver_install(i2c_master_port, conf.mode,
// // 										 I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
// // 										 I2C_EXAMPLE_MASTER_TX_BUF_DISABLE,
// // 										 0);
// //
// //   ProbeJoystick();
// //
// // #endif
// //
// //   printf("JoystickInit: done.\n");
// //
// // }
