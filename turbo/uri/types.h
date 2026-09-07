// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <string>
#include <cstdint>

namespace turbo {

    enum class UriHostType : uint8_t {
        /// Represents common URLs such as "https://www.google.com"
        DEFAULT = 0,
         /// Represents ipv4 addresses such as "http://127.0.0.1"
        IPV4 = 1,
         /// Represents ipv6 addresses such as
         /// "http://[2001:db8:3333:4444:5555:6666:7777:8888]"
        IPV6 = 2,
    };

    enum class StandType {
        STD_NONE,
        STD_RFC,
        STD_WPT,
    };

    enum class EnodeType {
        PLAIN,
        PRECENT
    };
}  // namespace turbo
