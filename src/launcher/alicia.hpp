#ifndef ALICIA_HPP
#define ALICIA_HPP

#include <string>
#include <cstdint>
#include <filesystem>

#include <windows.h>

namespace alicia
{

//! Web launch information.
struct WebInfo
{
  enum class Sex {
    UNSPECIFIED, FEMALE, MALE
  };

  std::string gameId;
  uint64_t memberNo{0};
  std::string loginId;
  std::string authKey;
  std::string installUrl;
  uint32_t serverType;
  std::string serverInfo;
  uint32_t age;
  Sex sex;
  std::string birthday;
  uint32_t wardNo;
  uint32_t cityCode;
  std::string zipCode;
  uint32_t pcBangNo;
  std::string closeTime;
};

//! Hosts a launch web information for the game.
class WebInfoHost
{
public:
  //! Default constructor.
  WebInfoHost() = default;

  //! Default destructor.
  ~WebInfoHost();

  //! Hosts the provided web info.
  //!
  //! @param webInfoId Web info Id.
  //! @param webInfo Web info.
  void host(std::string webInfoId, WebInfo& webInfo);

private:
  void create();
  void destroy();

  //! A handle to web info.
  std::string _webInfoId;
  //! A web info.
  WebInfo _webInfo{};

  //! A handle to the web info file.
  HANDLE _webInfoMappingHandle = nullptr;
};

class ModHost {
public:
  void loadMod(std::filesystem::path path);
};

} // namespace alicia

#endif //ALICIA_HPP
