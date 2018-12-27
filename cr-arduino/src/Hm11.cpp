#include "Hm11.hpp"

void Hm11::setBleState(BleState new_state) {
    if (new_state != state_) {
        switch (new_state) {
            case BS_Standby:
                digitalWrite(LED_BUILTIN, LOW);
                BLE_DEBUG("BLE awake");
                break;
            case BS_Connected:
                digitalWrite(LED_BUILTIN, HIGH);
                BLE_DEBUG("BLE connected");
                break;
            case BS_SleepRequested:
                BLE_DEBUG("BLE sleep requested");
                break;
            case BS_Sleep:
                BLE_DEBUG("BLE asleep");
                break;
            case BS_WakeRequested:
                BLE_DEBUG("BLE wake requested");
                break;
        }

        state_ = new_state;
        last_state_change_ = millis();
    }
}

Hm11::Hm11() : serial_(BTH_RX, BTH_TX), state_(BS_Standby), buffer_(), last_state_change_(0) {}

static Hm11 *hm11_;
extern void handleBleChar(uint8_t c) { hm11_->buffer_.push_over(c); }

void Hm11::begin(uint16_t baudrate) {
    // Set global ptr
    hm11_ = this;

    // RESET pin
    pinMode(BTH_RST, OUTPUT);

    // Prepare serial
    serial_.attachInterrupt(handleBleChar);
    serial_.begin(baudrate);
}

void Hm11::reset() {
    // RESET sequence on BTH
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(BTH_RST, LOW);
    delay(75);
    digitalWrite(BTH_RST, HIGH);
    digitalWrite(LED_BUILTIN, LOW);

    // Wait for module to boot
    delay(500);

    // Update state
    last_state_change_ = millis();
    state_ = BS_Standby;
}

void Hm11::poll(MessageHandler handler) {
    while (buffer_.size() > 3) {
        if (buffer_[0] == 'O' &&
            buffer_[1] == 'K' &&
            buffer_[2] == '+') {
            // OK+ : AT notification from module

            // Read CONN or LOST
            if (buffer_.size() >= 7 &&
                buffer_[3] == 'C' &&
                buffer_[4] == 'O' &&
                buffer_[5] == 'N' &&
                buffer_[6] == 'N')
            {
                setBleState(BS_Connected);
                buffer_.erase_front(7);
            } else if (buffer_.size() >= 7 &&
                       buffer_[3] == 'L' &&
                       buffer_[4] == 'O' &&
                       buffer_[5] == 'S' &&
                       buffer_[6] == 'T') {
                setBleState(BS_Standby);
                buffer_.erase_front(7);
            } else if (buffer_.size() >= 8 &&
                       buffer_[3] == 'S' &&
                       buffer_[4] == 'L' &&
                       buffer_[5] == 'E' &&
                       buffer_[6] == 'E') {
                setBleState(BS_Sleep);
                buffer_.erase_front(8);
            } else if (buffer_.size() >= 7 &&
                       buffer_[3] == 'W' &&
                       buffer_[4] == 'A' &&
                       buffer_[5] == 'K' &&
                       buffer_[6] == 'E') {
                setBleState(BS_Standby);
                buffer_.erase_front(7);
            }
        } else if (buffer_[0] == 'C' &&
                   buffer_[1] == 'R' &&
                   buffer_[2] == '+') {
            // CR+ : Canon Remote command
            handler(buffer_);
        }
    }

    if (state_ == BS_WakeRequested) 
    {
        serial_.print(F("Please wake up little module"));
    }
}

void Hm11::set_name(const char *name) {
    serial_.print(String("AT+NAME") + String(name));
    delay(50);
    buffer_.clear();
}

bool Hm11::sleep() {
    if (state_ == BS_Standby) {
        setBleState(BS_SleepRequested);
        serial_.write("AT+SLEEP");

        return true;
    }

    return false;
}

bool Hm11::wake() {
    if (state_ == BS_Sleep) {
        setBleState(BS_WakeRequested);

        return true;
    }

    return false;
}
