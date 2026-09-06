// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/uri/uri_view.h>
#include <turbo/strings/substring.h>
namespace turbo {

    namespace {
        std::string_view component_slice(std::string_view data,
            const std::optional<ComponentView> &comp) {
            if (!comp.has_value()) {
                return "";
            }
            return subview(data, comp.value().start, comp.value().end);
        }
    }  // namespace

    UriView::UriView(std::string &&s)
        : _type(StandType::STD_NONE), _encode(EnodeType::PLAIN) {
        _store = std::move(s);
        _uri_data = *_store;
    }

    UriView::UriView(std::string_view s)
        : _uri_data(s),
          _type(StandType::STD_NONE),
          _encode(EnodeType::PLAIN) {}

    std::string_view UriView::shema() const {
        return component_slice(_uri_data, _schema);
    }

    bool UriView::ok() const noexcept {
        return _error.ok();
    }

    StandType UriView::standard() const noexcept {
        return _type;
    }

    EnodeType UriView::encode_type() const noexcept {
        return _encode;
    }

    const UriError& UriView::uri_error() const noexcept {
        return _error;
    }

    std::string_view UriView::host() const {
        return component_slice(_uri_data, _host);
    }

    std::string_view UriView::port() const {
        return component_slice(_uri_data, _port);
    }

    std::string_view UriView::path() const {
        return component_slice(_uri_data, _path);
    }

    std::string_view UriView::query() const {
        return component_slice(_uri_data, _query);
    }

    std::string_view UriView::fragment() const {
        return component_slice(_uri_data, _fragment);
    }

    const QueryParams& UriView::query_params() const noexcept {
        static const QueryParams kEmpty;
        if (!_query_params.has_value()) {
            return kEmpty;
        }
        return *_query_params;
    }

    EnodeType& UriView::encode_type(EnodeType et) {
        _encode = et;
        return _encode;
    }

    StandType& UriView::standard(StandType st) {
        _type = st;
        return _type;
    }

    UriError& UriView::uri_error(UriError err) {
        _error = std::move(err);
        return _error;
    }

    void UriView::shema(ComponentView view) {
        _schema = view;
    }

    void UriView::host(ComponentView view) {
        _host = view;
    }

    void UriView::port(ComponentView view) {
        _port = view;
    }

    void UriView::path(ComponentView view) {
        _path = view;
    }

    void UriView::query(ComponentView view) {
        _query = view;
    }

    void UriView::fragment(ComponentView view) {
        _fragment = view;
    }

    void UriView::query_params(QueryParams params) {
        _query_params = std::move(params);
    }

    void UriView::reset_shema() {
        _schema = std::nullopt;
    }

    void UriView::reset_host() {
        _host = std::nullopt;
    }

    void UriView::reset_port() {
        _port = std::nullopt;
    }

    void UriView::reset_path() {
        _path = std::nullopt;
    }

    void UriView::reset_query() {
        _query = std::nullopt;
    }

    void UriView::reset_fragment() {
        _fragment = std::nullopt;
    }

    void UriView::reset_query_params() {
        _query_params = std::nullopt;
    }

    void UriView::reset() {
        _uri_data = {};
        _store.reset();
        _error = {};
        _schema = std::nullopt;
        _host = std::nullopt;
        _port = std::nullopt;
        _path = std::nullopt;
        _query = std::nullopt;
        _fragment = std::nullopt;
        _query_params = std::nullopt;
    }
}  // namespace turbo
