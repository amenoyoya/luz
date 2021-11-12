#define _USE_LUZ_CORE
#define _USE_LUZ_ZIP
#define _USE_LUZ_LUA
#include <luz/zip.hpp>
#include <luz/lua.hpp>

/// get current file content in the zip data
inline std::string unz_getcontent(unz_archiver_t *unz) {
    unz_file_info_t info;
    if (!unz_info(unz, &info, nullptr, 0, nullptr, 0)) return "";

    std::string content;
    content.resize(info.uncompressed_size);
    if (!unz_content(unz, (char*)content.c_str(), info.uncompressed_size, nullptr)) return "";
    return std::move(content);
}

__main() {
    /// prepare Luz engine
    sol::state lua;
    std::string errorMessage;
    if (!lua_registlib(lua, &errorMessage)) {
        _fputs(stderr, errorMessage);
        return 1;
    }

    /// os.argv <= args (std::vector<std::string(UTF-8)>)
    auto os = lua["os"].get_or_create<sol::table>();
    os["argv"] = sol::as_table(args);

    // main script: main.sym in this execution file resource
    unz_archiver_t *unz = unz_open(args[0].c_str());

    if (!unz) {
        _fputs(stderr, "Luz has no resource: '" + args[0] + "'");
        return 1;
    }
    if (!unz_locate_name(unz, "main.sym")) {
        _fputs(stderr, "No main script found in '" + args[0] + "'");
        unz_close(unz);
        return 1;
    }

    std::string buffer = unz_getcontent(unz);
    auto script = lua.load_buffer((const char *)buffer.c_str(), buffer.size(), "@" + args[0]);

    unz_close(unz);
    if (!script.valid()) {
        sol::error err = script;
        _fputs(stderr, strtou8(err.what()));
        return 1;
    }

    auto result = script();
    if (!result.valid()) {
        sol::error err = result;
        _fputs(stderr, strtou8(err.what()));
        return 1;
    }
    return 0;
}
