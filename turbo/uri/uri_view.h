// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <turbo/uri/types.h>
#include <turbo/uri/uri_error.h>

namespace turbo {

    //////////////////////////////////////////////////
    /// setters privode for paser or other builtin
    /// and not for user api
    struct ComponentView {
        static constexpr uint32_t npos = std::numeric_limits<uint32_t>::max();
        uint32_t start{npos};
        uint32_t end{npos};
    };

    using QueryParams = std::vector<std::pair<ComponentView,ComponentView>>;

    class UriView {
    public:
        UriView(const UriView& u) = default;
        UriView(UriView&& u) noexcept = default;
        UriView& operator=(UriView&& u) noexcept = default;
        UriView& operator=(const UriView& u) = default;
        ~UriView() = default;

        ///////////////////////
        /// getters
        [[nodiscard]] bool ok() const noexcept;
        [[nodiscard]] StandType standard() const noexcept;
        [[nodiscard]] EnodeType encode_type() const noexcept;
        [[nodiscard]] const UriError& uri_error() const noexcept;

        std::string_view shema() const;
        std::string_view host() const;
        std::string_view port() const;
        std::string_view path() const;
        std::string_view query() const;
        std::string_view fragment() const;
        [[nodiscard]] const QueryParams& query_params() const noexcept;

        ///////////////////////
        /// checkers

    private:
        UriView(std::string &&);
        UriView(std::string_view);
        ///////////////////////
        /// cleaner
        void reset();
        void reset_shema();
        void reset_host();
        void reset_port();
        void reset_path();
        void reset_query();
        void reset_fragment();
        void reset_query_params();
        ///////////////////////
        /// setters
        EnodeType& encode_type(EnodeType et);
        StandType& standard(StandType st);
        UriError& uri_error(UriError err);
        void shema(ComponentView view);
        void host(ComponentView view);
        void port(ComponentView view);
        void path(ComponentView view);
        void query(ComponentView view);
        void fragment(ComponentView view);
        void query_params(QueryParams params);
    protected:
        std::optional<std::string> _store;
        std::string_view         _uri_data;
        StandType                    _type;
        EnodeType                    _encode;
        UriError                     _error;

        std::optional<ComponentView> _schema;
        std::optional<ComponentView> _host;
        std::optional<ComponentView> _port;
        std::optional<ComponentView> _path;
        std::optional<ComponentView> _query;
        std::optional<ComponentView> _fragment;
        std::optional<QueryParams>   _query_params;
    };
}  // namespace turbo
