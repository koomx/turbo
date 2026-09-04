#include <turbo/uri/json.h>
#include <turbo/uri/uri_components.h>
#include <string>

namespace turbo {

    // Only valid after refine_to().
    [[nodiscard]] bool UriComponents::check_offset_consistency() const noexcept {
        if (schema.is_null() || !schema.is_valid()) {
            return false;
        }
        uint32_t index = schema.end;

        auto step = [&](const UriComponent &c) -> bool {
            if (c.is_null() || !c.is_valid()) {
                return false;
            }
            if (c.start < index) {
                return false;
            }
            index = c.end;
            return true;
        };

        if (!step(username)) {
            return false;
        }
        if (!step(password)) {
            return false;
        }
        if (!step(host)) {
            return false;
        }
        if (!step(port)) {
            return false;
        }
        if (!step(pathname)) {
            return false;
        }
        if (!step(query)) {
            return false;
        }
        if (!step(fragment)) {
            return false;
        }
        return true;
    }

    [[nodiscard]] std::string UriComponents::to_string() const {
        std::string answer;
        auto back = std::back_insert_iterator(answer);
        answer.append("{\n");

        auto append_comp = [&](const char *name, const UriComponent &c) {
            answer.append("\t\"");
            answer.append(name);
            answer.append("\":{\"start\":");
            encode_json(std::to_string(c.start), back);
            answer.append(",\"end\":");
            encode_json(std::to_string(c.end), back);
            answer.append("},\n");
        };

        append_comp("schema", schema);
        append_comp("username", username);
        append_comp("password", password);
        append_comp("host", host);
        append_comp("port", port);
        append_comp("pathname", pathname);
        append_comp("query", query);
        append_comp("fragment", fragment);

        answer.append("\n}");
        return answer;
    }

    bool UriComponents::refine_to(std::string_view src, std::string &dst,
        UriComponents &dst_com) const {
        // Fill only null slots in dst_com from *this; never replace. No base merge.
        auto fill = [&](const UriComponent &from, UriComponent &to) {
            if (!to.is_null()) {
                return;
            }
            if (from.is_null()) {
                to.set_empty(static_cast<uint32_t>(dst.size()));
                return;
            }
            to = from;
            to.refine_to(src, &dst);
        };

        const bool write_schema = dst_com.schema.is_null();
        if (write_schema) {
            fill(schema, dst_com.schema);
            if (!dst_com.schema.empty()) {
                dst.push_back(':');
            }
        }

        const bool write_host = dst_com.host.is_null();
        if (write_host) {
            if (!host.is_null() && !host.empty()) {
                dst.append("//");
                if (dst_com.username.is_null()) {
                    fill(username, dst_com.username);
                    if (!dst_com.username.empty() || (!password.is_null() && !password.empty())) {
                        if (dst_com.password.is_null()) {
                            if (!password.is_null() && !password.empty()) {
                                dst.push_back(':');
                            }
                            fill(password, dst_com.password);
                        }
                        dst.push_back('@');
                    } else if (dst_com.password.is_null()) {
                        fill(password, dst_com.password);
                    }
                } else if (dst_com.password.is_null()) {
                    fill(password, dst_com.password);
                }
                fill(host, dst_com.host);
                if (dst_com.port.is_null()) {
                    if (!port.is_null() && !port.empty()) {
                        dst.push_back(':');
                    }
                    fill(port, dst_com.port);
                }
            } else {
                fill(host, dst_com.host);
                if (dst_com.username.is_null()) {
                    fill(username, dst_com.username);
                }
                if (dst_com.password.is_null()) {
                    fill(password, dst_com.password);
                }
                if (dst_com.port.is_null()) {
                    fill(port, dst_com.port);
                }
            }
        } else {
            if (dst_com.username.is_null()) {
                fill(username, dst_com.username);
            }
            if (dst_com.password.is_null()) {
                fill(password, dst_com.password);
            }
            if (dst_com.port.is_null()) {
                fill(port, dst_com.port);
            }
        }

        if (dst_com.pathname.is_null()) {
            fill(pathname, dst_com.pathname);
        }

        if (dst_com.query.is_null()) {
            if (!query.is_null() && !query.empty()) {
                dst.push_back('?');
            }
            fill(query, dst_com.query);
        }

        if (dst_com.fragment.is_null()) {
            if (!fragment.is_null() && !fragment.empty()) {
                dst.push_back('#');
            }
            fill(fragment, dst_com.fragment);
        }

        return true;
    }

}  // namespace turbo
