#include "es8388.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"

// Note: These would ideally come from a board config component, 
// but we define them here to match the Stinger pin map for Phase 1.
#define I2C_MASTER_SCL_IO           5
#define I2C_MASTER_SDA_IO           4
#define I2C_MASTER_NUM              0
#define ES8388_I2C_ADDR             0x10

static const char *TAG = "ES8388";
static i2c_master_dev_handle_t s_dev_handle;

/**
 * @brief Write a single byte to an ES8388 register
 */
static esp_err_t es_write_reg(uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(s_dev_handle, write_buf, sizeof(write_buf), -1);
}

/**
 * @brief Initialize the I2C master bus and device handle
 */
static esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_bus_config, &bus_handle), TAG, "I2C master bus init failed");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ES8388_I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus_handle, &dev_cfg, &s_dev_handle), TAG, "I2C device add failed");

    return ESP_OK;
}

esp_err_t es8388_init(void) {
    ESP_LOGI(TAG, "Initializing ES8388 Codec...");
    
    // 1. Initialize I2C Bus
    ESP_RETURN_ON_ERROR(i2c_master_init(), TAG, "I2C Init Failed");

    // 2. Hardware Resident Sequence (Based on ES8388 Datasheet/ESP-ADF)
    // -----------------------------------------------------------------
    // Reset and basic clocking
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_CONTROL1, 0x00), TAG, "Reg 0x00 failed");
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_CONTROL2, 0x00), TAG, "Reg 0x01 failed");
    
    // Chip Power management (DAC & ADC Power Up)
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_CHIPPOWER, 0x00), TAG, "Power up failed");
    
    // Master/Slave mode (Set to Slave, ESP32-S3 is Master)
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_MASTER_MODE, 0x00), TAG, "Master mode failed");
    
    // ADC/DAC Serial Interface Format (I2S, 16-bit)
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_ADC_CONTROL1, 0x00), TAG, "ADC Ctrl 1 failed");
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_DAC_CONTROL1, 0x00), TAG, "DAC Ctrl 1 failed");
    
    // Output Routing and Gain
    // Set DAC Output to LOUT1/ROUT1 (Headphones)
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_DAC_CONTROL17, 0x90), TAG, "DAC Ctrl 17 failed"); // Enable LOUT1/ROUT1
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_DAC_CONTROL20, 0x90), TAG, "DAC Ctrl 20 failed"); // Enable LOUT2/ROUT2 (Optional)
    
    // Unmute
    ESP_RETURN_ON_ERROR(es8388_set_voice_volume(70), TAG, "Initial volume set failed");

    ESP_LOGI(TAG, "ES8388 initialized successfully.");
    return ESP_OK;
}

esp_err_t es8388_set_voice_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    // Scale 0-100 to 0x00-0x21 (approx register range for ES8388 DAC volume)
    // In many implementations, 0x00 is max (0dB) and 0x21 is min (-96dB)
    // To keep it simple for now, we use the standard 0-33 range.
    uint8_t vol_reg = (uint8_t)((100 - volume) * 33 / 100);
    
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_DAC_CONTROL24, vol_reg), TAG, "Vol L failed");
    ESP_RETURN_ON_ERROR(es_write_reg(ES8388_DAC_CONTROL25, vol_reg), TAG, "Vol R failed");
    
    return ESP_OK;
}
