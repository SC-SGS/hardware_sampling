/**
 * @author Marcel Breyer
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 */

#include "hardware_sampling/event.hpp"

#include <format>   // std::format
#include <ostream>  // std::ostream

namespace hws {

std::ostream &operator<<(std::ostream &out, const event &e) {
    return out << std::format("time_point: {}\n"
                              "name: {}",
                              e.time_point.time_since_epoch(),
                              e.name);
}

}  // namespace hws
