#include <filesystem>

#include "nlohmann/json.hpp"

namespace fs = std::filesystem;

class Asset;
class Deployment;

// Visibility is set to default so that the function is visible in binder as well.
// This functions is implemented in binder as it needs pybind11, which is not linked with nimblenet
__attribute__((visibility("default"))) std::string parseScriptToAST(const std::string& scriptPath);

class SimulatorUtils {
 public:
  __attribute__((visibility("default"))) static void copy_modules(nlohmann::json&& moduleConfig);

 private:
  static void copy_module(const std::shared_ptr<Asset> asset, Deployment& deployment,
                          bool addToDeployment);
  static void create_symlink(const fs::path& target, const std::string& link);
};
