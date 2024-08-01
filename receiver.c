#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_wifi.h"
#include "esp_mac.h"
#include "espnow_utils.h"
#include "espnow.h"
#include "espnow_ctrl.h"
#include "driver/gpio.h"
#include "espnow_mem.h"

#include <sys/time.h> // Montok hiza / for time

//#include "build/config/sdkconfig.h" // Posuango popianai do intellisense nopo, pokinomio' pogulu mamaal
/* Only to silence intellisense nagging, comment before building */

#define KOTOS_1        2 // GPIO2, D0 id dulak / on board
#define KOTOS_2        3 // GPIO3, D1 id dulak / on board
#define KOTOS_3        4 // GPIO4, D2 id dulak / on board

#define BIRI_1         5 // GPIO5, D3 id dulak / on board
#define BIRI_2         6 // GPIO6, D4 id dulak / on board
#define BIRI_3         7 // GPIO7, D5 id dulak / on board

typedef struct gamit {
    int buttonstate1;
    int buttonstate2;
    int buttonstate3;
    int buttonstate4;
    int buttonstate5;
    int buttonstate6;

} gamit;
gamit dolinon; // Mamadalin do poiloon id pampos Koromitan / For transferring data to Receiving function

static const char *TAG = "sodusuhu";

void set_time(void) {
    struct timeval now;
    now.tv_sec = 1722541840; // Seconds since Unix epoch / Kiop tantad tinontok Unix
    now.tv_usec = 0;

    settimeofday(&now, NULL);
}

char* get_time(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return asctime(tm);
}

static void ponimpuun_wifi() // WiFi init
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void ponimpuun_kotos(void)
{
    gpio_reset_pin(KOTOS_1);
    gpio_reset_pin(KOTOS_2);
    gpio_reset_pin(KOTOS_3);

    /* Tolu GPIO hiti nopo nga' pinisok / These three GPIOs are buttons */
    gpio_set_direction(KOTOS_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(KOTOS_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(KOTOS_3, GPIO_MODE_OUTPUT);

    /* GPIO pomiri don tikid GPIO pinisok / Reader GPIOs for each GPIO buttons */
    gpio_set_direction(BIRI_1, GPIO_MODE_INPUT);
    gpio_set_direction(BIRI_2, GPIO_MODE_INPUT);
    gpio_set_direction(BIRI_3, GPIO_MODE_INPUT);

    gpio_set_level(KOTOS_1, 0);
    gpio_set_level(KOTOS_2, 0);
    gpio_set_level(KOTOS_3, 0);
}

void Koromitan(const uint8_t *pagatadan, const uint8_t *poiloon_mikot, int ninaru) {
    memcpy(&dolinon, poiloon_mikot, sizeof(dolinon));
    if (dolinon.buttonstate1 == 1) {
        gpio_set_level(KOTOS_1, !gpio_get_level(BIRI_1));
        ESP_LOGI(TAG, "Button 1 signal received at: %s", get_time());
    }
    
    if (dolinon.buttonstate2 == 1) {
        gpio_set_level(KOTOS_2, !gpio_get_level(BIRI_2));
        ESP_LOGI(TAG, "Button 2 signal received at: %s", get_time());
    }

    if (dolinon.buttonstate3 == 1) {
        gpio_set_level(KOTOS_3, !gpio_get_level(BIRI_3));
        ESP_LOGI(TAG, "Button 3 signal received at: %s", get_time());
    }

}

void app_main(void)
{
    for (int i = 0; i < 1; i++) {
        ponimpuun_kotos();
        set_time();
    }

    espnow_storage_init();
    ponimpuun_wifi();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);
    ESP_ERROR_CHECK(esp_now_register_recv_cb(Koromitan));
    ESP_LOGI(TAG, "Receive callback registered");
}
