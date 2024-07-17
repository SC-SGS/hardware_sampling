/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/hardware_sampler.hpp"

#include <chrono>     // std::chrono::{steady_clock, duration_cast, milliseconds}
#include <exception>  // std::exception
#include <exception>  // std::exception
#include <iostream>   // std::cerr, std::endl
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <thread>     // std::thread

namespace hws {

hardware_sampler::hardware_sampler(const std::chrono::milliseconds sampling_interval) :
    sampling_interval_{ sampling_interval } { }

hardware_sampler::~hardware_sampler() = default;

void hardware_sampler::start_sampling() {
    // can't start an already running sampler
    if (this->has_sampling_started()) {
        throw std::runtime_error{ "Can start every hardware sampler only once!" };
    }

    // start sampling loop
    sampling_started_ = true;
    this->resume_sampling();
    sampling_thread_ = std::thread{
        [this]() {
            try {
                this->sampling_loop();
            } catch (const std::exception &e) {
                // print useful error message
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
    };
}

void hardware_sampler::stop_sampling() {
    // can't stop a hardware sampler that has never been started
    if (!this->has_sampling_started()) {
        throw std::runtime_error{ "Can't stop a hardware sampler that has never been started!" };
    }
    // can't stop an already stopped sampler
    if (this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can stop every hardware sampler only once!" };
    }

    // stop sampling
    this->pause_sampling();
    sampling_stopped_ = true;  // -> notifies the sampling std::thread
    sampling_thread_.join();
}

void hardware_sampler::pause_sampling() {
    sampling_running_ = false;  // notifies the sampling std::thread
}

void hardware_sampler::resume_sampling() {
    if (this->has_sampling_stopped()) {
        throw std::runtime_error{ "Can't resume a hardware sampler that has already been stopped!" };
    }
    sampling_running_ = true;  // notifies the sampling std::thread
}

bool hardware_sampler::has_sampling_started() const noexcept {
    return sampling_started_;
}

bool hardware_sampler::is_sampling() const noexcept {
    return sampling_running_;
}

bool hardware_sampler::has_sampling_stopped() const noexcept {
    return sampling_stopped_;
}

}  // namespace hws
