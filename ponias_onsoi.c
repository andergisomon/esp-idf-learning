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
// #include "build/config/sdkconfig.h" // Posuango popianai do intellisense nopo, pokinomio' pogulu mamaal

#define KOTOS_1        2 // GPIO2, D0 id dulak
#define KOTOS_2        3 // GPIO3, D1 id dulak
#define KOTOS_3        4 // GPIO4, D2 id dulak

int prev1 = 0; int prev2 = 0; int prev3 = 0;
int current1 = 0; int current2 = 0; int current3 = 0;

uint8_t pagatadan[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const char *TAG = "sodusuhu";

typedef struct gamit {
    int buttonstate1;
    int buttonstate2;
    int buttonstate3;
    int buttonstate4;
    int buttonstate5;
    int buttonstate6;

} gamit;
gamit dolinon; // Mamadalin do poiloon id pampos Poniasan


static void ponimpuun_wifi()
{
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void ponimpuun_pomiri(void)
{
    gpio_reset_pin(KOTOS_1);
    gpio_reset_pin(KOTOS_2);
    gpio_reset_pin(KOTOS_3);

    /* Tolu GPIO hiti nopo nga' pinisok */
    gpio_set_direction(KOTOS_1, GPIO_MODE_INPUT);
    gpio_set_direction(KOTOS_2, GPIO_MODE_INPUT);
    gpio_set_direction(KOTOS_3, GPIO_MODE_INPUT);
}

// This function is formerly uint8_t void
static void pomiri(void) {
    current1 = gpio_get_level(KOTOS_1);
    current2 = gpio_get_level(KOTOS_2);
    current3 = gpio_get_level(KOTOS_3);

    if (prev1 == 0 && current1 == 1) {
        dolinon.buttonstate1 = 1;    
    }
    else {
        dolinon.buttonstate1 = 0;
    }

    if (prev2 == 0 && current2 == 1) {
        dolinon.buttonstate2 = 1;    
    }
    else {
        dolinon.buttonstate2 = 0;
    }

    if (prev3 == 0 && current3 == 1) {
        dolinon.buttonstate3 = 1;    
    }
    else {
        dolinon.buttonstate3 = 0;
    }
}

static void poniasan(void *arg) {
    esp_err_t sinuli  = ESP_OK;

    while (1) {
        pomiri();
        vTaskDelay(50 / portTICK_PERIOD_MS);
        esp_now_send(pagatadan, (uint8_t *) &dolinon, sizeof(dolinon));
        printf("\n%i\n", sinuli);
        printf("Switch 1: %d", dolinon.buttonstate1);
        printf("Switch 2: %d", dolinon.buttonstate2);
        printf("Switch 3: %d", dolinon.buttonstate3);
        prev1 = current1; prev2 = current2; prev3 = current3;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Poniasan nakalabus");
    vTaskDelete(NULL);
}

static void ponimpuun_ponias(void) {
    xTaskCreate(poniasan, "Popotimpuun ponias", 4 * 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}


void app_main(void)
{
    ponimpuun_pomiri();
    espnow_storage_init();

    ponimpuun_ponias();
    ponimpuun_wifi();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);
}
