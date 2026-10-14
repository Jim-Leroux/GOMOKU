#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
    std::string directoryOf(const char* argv0)
    {
        char path[4096];

#if defined(__linux__)
        const ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len > 0)
        {
            path[len] = '\0';
            std::string exe(path);
            const std::size_t slash = exe.find_last_of('/');
            return slash == std::string::npos ? "." : exe.substr(0, slash);
        }
#endif

        if (argv0 && realpath(argv0, path))
        {
            std::string exe(path);
            const std::size_t slash = exe.find_last_of('/');
            return slash == std::string::npos ? "." : exe.substr(0, slash);
        }

        return ".";
    }

    int runShell(const char* cmd)
    {
        const int status = std::system(cmd);
        if (status == -1)
        {
            std::cerr << "Gomoku: command failed: " << std::strerror(errno) << "\n";
            return 1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            return WEXITSTATUS(status);
        if (!WIFEXITED(status))
            return 1;
        return 0;
    }
}

int main(int argc, char** argv)
{
    (void)argc;

    const std::string root = directoryOf(argv[0]);
    if (chdir(root.c_str()) != 0)
    {
        std::cerr << "Gomoku: cannot enter project directory: " << std::strerror(errno) << "\n";
        return 1;
    }

    std::cout << "Building WASM with Docker for local dev...\n";
    if (runShell(
            "if docker --version 2>/dev/null | grep -qi podman; then "
            "docker run --rm -v \"$(pwd):/src:z\" emscripten/emsdk make wasm; "
            "else "
            "docker run --rm -v \"$(pwd):/src:z\" -u $(id -u):$(id -g) emscripten/emsdk make wasm; "
            "fi") != 0)
        return 1;

    std::cout << "Copying WASM files to React public folder...\n";
    if (runShell("cp gomoku.js gomoku.wasm web/public/") != 0)
        return 1;

    std::cout << "Starting React development server...\n";
    execlp("npm", "npm", "--prefix", "web", "run", "dev", nullptr);

    std::cerr << "Gomoku: failed to start npm: " << std::strerror(errno) << "\n";
    return 1;
}
