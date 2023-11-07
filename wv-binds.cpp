
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#include "Utf8Conv.hpp"
#include "wv-winapi.h"
using Utf8Conv::Utf16ToUtf8;
using Utf8Conv::Utf8ToUtf16;
#include <windows.h>
#endif

#include "base64.hpp"
#include "wv-curl.h"
#include "wv-util.h"
#include "wv-wrap.h"

webview_wrapper w;

// Convert a wstring to a string containing a suite of hexa numbers, separated
// by space and representing the utf code of each characters of the wstring
// i.e. the wstring "totö要らない" will result in a string "74 6f 74 f6 8981
// 3089 306a 3044"
std::string s_w2h(std::wstring ws)
{
  std::stringstream cnv;
  for (auto e : ws)
  {
    // if (cnv.tellp() != 0)
    if (!cnv.str().empty())
      cnv << ' ';
    cnv << std::hex << (unsigned int)e;
  }

  return cnv.str();
}

// Convert a string containing a suite of hexa numbers, separated by space and
// representing utf code to a wstring i.e. the string "74 6f 74 f6 8981 3089
// 306a 3044"  will give the wstring "totö要らない"
std::wstring s_h2w(std::string hs)
{
  std::string hex_chars(hs);

  std::istringstream hex_chars_stream(hex_chars);
  std::wstring ws=L"";
  unsigned int c;
  while (hex_chars_stream >> std::hex >> c)
  {
    ws += (wchar_t)c;
  }

  return ws;
}

void write_cons(std::string s, std::ostream &out=std::cout)
{
  if (s.empty())
    return;
#ifdef _WIN32
  char title[256];
  std::string tit="";
  int gctres=GetConsoleTitle(title, 255);
  // out << gctres << std::endl;
  if (gctres > 0)
  {
    tit=title;
    if (tit.find("invisible cygwin console") != std::string::npos)
    {
      out << s;
      out.flush();
    }
    else
    {
      HANDLE ho;
      if (&out == &std::cerr)
        ho=GetStdHandle(STD_ERROR_HANDLE);
      else
        ho=GetStdHandle(STD_OUTPUT_HANDLE);
      WriteConsole(ho, s.c_str(), (DWORD)s.size(), nullptr, nullptr);
    }
  }
  else
    out << s;
  out.flush();
#else
  out << s;
  out.flush();
#endif

  w.eval("console.log('[[" + s + "]]');");
}

bool isWideString(const std::string s)
{
  for (auto &c : s)
  {
    // if(c & 0x80) return true;
    if (c > 0xff)
      return true;
  }

  return false;
}

bool isWideString(const std::wstring s)
{
  for (auto &c : s)
  {
    // if(c & 0x80) return true;
    // if (c > 0xff)
    if (c > 0x80)
      return true;
  }

  return false;
}

std::string skipWideChars(std::string ws)
{
  std::string s;
  for (auto &c : ws)
  {
    // if (c & 0x80) s+='_';
    if (c > 0xff)
      s += '_';
    else
      s += (char)c;
  }

  return s;
}

std::string skipWideChars(std::wstring ws)
{
  std::string s;
  for (auto &c : ws)
  {
    // if(c & 0x80) s+='*';
    if (c > 0xff)
      s += '_';
    else
      s += (char)c;
  }

  return s;
}

std::string setfile(std::filesystem::path path) {
	std::cout << path << std::endl;
  std::string eps;
  std::cout << 0 <<std::endl;
  eps=to_htent(path.wstring());
  std::cout << 1 <<std::endl;
  replace_all(eps, "\\", "/");
  std::cout << 2 <<std::endl;
  eps="\"path\":\"" + eps + '"';
  std::cout << 3 <<std::endl;
  return "{" + eps + "},";
}

const std::string hexa_pfx="UTF_IN_HEXA_STRING";
// Return a javascript array of strings corresponding to a directory list,
// recursively or not
std::string lsdir(std::string spath, bool recursive=false)
{
  std::string res="", res_line;
  std::filesystem::path path;

  if (spath.empty()) path=".";
  else path=spath;

  if (std::filesystem::is_directory(path)) {
    // Pas de récursif, trop dangereux, ça plante le PC ...
    for (const auto &e : std::filesystem::directory_iterator(path))
    {
      res += setfile(e);
    }
  } else {
    if (std::filesystem::is_regular_file(path)) {
      res += setfile(path.filename());
    } else
      res += setfile("not a directory or file '"+path.filename().string()+"'");
  }

  // Remove last comma
  res.pop_back();

  res="{\"result\":[" + res + "]}";
  return res;
}

// Return a string that represents the number in hexadecimal form
std::string to_js_hex(unsigned int number)
{
  std::ostringstream str;
  str << std::hex << number;
  return "0X" + str.str();
}

// Return a string that represents the number in octal form
std::string to_js_oct(unsigned int number)
{
  std::ostringstream str;
  str << std::oct << number;
  return "0" + str.str();
}

void print_file_types()
{
  static int un=true;
  if (un)
  {
    un=false;
    std::cout << "none      :" << (int)std::filesystem::file_type::none << std::endl;
    std::cout << "not_found :" << (int)std::filesystem::file_type::not_found << std::endl;
    std::cout << "regular   :" << (int)std::filesystem::file_type::regular << std::endl;
    std::cout << "directory :" << (int)std::filesystem::file_type::directory << std::endl;
    std::cout << "symlink   :" << (int)std::filesystem::file_type::symlink << std::endl;
    std::cout << "block     :" << (int)std::filesystem::file_type::block << std::endl;
    std::cout << "character :" << (int)std::filesystem::file_type::character << std::endl;
    std::cout << "fifo      :" << (int)std::filesystem::file_type::fifo << std::endl;
    std::cout << "socket    :" << (int)std::filesystem::file_type::socket << std::endl;
    std::cout << "unknown   :" << (int)std::filesystem::file_type::unknown << std::endl;
    std::cout << std::flush;
  }
}
// As file_type have unspecified values in the C++ standard
// and as I notice a difference between G++ and MSVC
// let force there value to my own choice for javascript
int forced_file_type(std::filesystem::file_type ft)
{
  switch (ft)
  {
  case std::filesystem::file_type::not_found:
    return 0;
  case std::filesystem::file_type::none:
    return 1;
  case std::filesystem::file_type::regular:
    return 2;
  case std::filesystem::file_type::directory:
    return 3;
  case std::filesystem::file_type::symlink:
    return 4;
  case std::filesystem::file_type::block:
    return 5;
  case std::filesystem::file_type::character:
    return 6;
  case std::filesystem::file_type::fifo:
    return 7;
  case std::filesystem::file_type::socket:
    return 8;
  case std::filesystem::file_type::unknown:
  default:
    return 9;
  }
}

std::string sec_wait(std::string ssec)
{
  auto msg=ssec + " second wait";
  int sec=std::stoi(ssec);
  //  std::cout << "Starting " << msg << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(sec));
  //  std::cout << "Ending   " << msg << std::endl;
  return '"' + msg + " is over.\"";
}

template <typename TP> std::time_t to_time_t(TP tp)
{
  namespace ch=std::chrono;
  auto sctp=ch::time_point_cast<ch::system_clock::duration>(tp - TP::clock::now() + ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
#define my_gmtime(a, b) gmtime_s(b, a)
#define my_localtime(a, b) localtime_s(b, a)
#else
#define my_gmtime(a, b) gmtime_r(a, b)
#define my_localtime(a, b) localtime_r(a, b)
#endif

// Convert a file time to a string, default format is ISO8601 and default time
// zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt="%Y-%m-%d %H:%M:%S",
                                bool gm=false)
{
  std::time_t tt=to_time_t(file_time);
  std::tm tim;
  if (gm)
    my_gmtime(&tt, &tim);
  else
    my_localtime(&tt, &tim);
  std::stringstream buffer;
  buffer << std::put_time(&tim, fmt.c_str());
  std::string fmtime=buffer.str();

  return fmtime;
}

std::string lastwrite(std::filesystem::path p)
{
  return file_time_to_string(std::filesystem::last_write_time(p));
}

bool isBase64(const std::string s)
{
  if (s.length() % 4 == 0)
    return std::regex_match(s, std::regex("^[A-Za-z0-9+/]*={0,2}$"));
  return false;
}

void fwrite(std::string fname, std::string s, std::ios_base::openmode omod=std::ios::app)
{
  std::ofstream f(fname, omod);
  f << s;
  f.close();
}

std::string do_fstat(std::string sp)
{
  std::wstring ws=htent_to_ws(sp);
  replace_all(ws, L"\\", L"/");
  std::filesystem::path p=ws;
//  std::cout << "do_fstat " << sp << "<=>";
//  std::wcout << ws;
//  std::cout << "<=>" << p.string() << std::endl;

  auto fs=std::filesystem::status(p);
  auto ft=fs.type();
  std::uintmax_t sz;
  if (ft == std::filesystem::file_type::regular)
    sz=std::filesystem::file_size(p);
  else
    sz=static_cast<std::uintmax_t>(-1);
  std::string lastwr="****-**-**T**:**:**";
  if (ft != std::filesystem::file_type::not_found)
    lastwr=lastwrite(p);
  else
  {
    std::cout << "NOT FOUND " << sp << std::endl;
  }
  std::string res="{\"file\":\"" + sp + "\"," + "\"type\":\"" + std::to_string(forced_file_type(ft)) + "\"," +
                    "\"perms\":\"" + to_js_oct((unsigned)fs.permissions()) + "\"," + "\"size\":\"" +
                    std::to_string(sz) + "\"," + "\"last_write\":\"" + lastwr + "\"}";
  // std::cout << "end fstat:" << res << std::endl << std::flush;
  return res;
}

// Synchronous bind
bool s2b(std::string s)
{
  if (s == "true")
    return true;
  return false;
}

// console.log(webapp_help())
static unsigned int nfs=0;
void create_binds(webview_wrapper &w)
{
  w.bind_doc(
      "echo", "echo the parameter.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto sp=json_parse(req, "", 0);
          std::string to_ht=to_htent(sp);
#ifdef _WIN32
          SetConsoleOutputCP(CP_UTF8);
#endif
//          std::cout << "param " << sp << std::endl;
//          std::cout << "to_htent " << to_ht << std::endl;
          std::string ht_to=htent_to_s(to_ht);
          std::cout << "htent_to " << htent_to_s(to_ht) << std::endl;
          w.resolve(seq, 0, '"' + to_ht + '"');
        }).detach();
      },
      nullptr);

  w.bind_doc(
      "utf", "return a random utf string.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto sp=json_parse(req, "", 0);
          std::cout << "utf param " << sp << std::endl;
          std::string s;
          if (sp == "1")
            s="totö要らない";
          else if (sp == "2")
            s="पार्सल् एक्स्प्रेस्";
          else if (sp == "3")
            s="123456788:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽž؀؁؂؃؄؅؆؇؈؉؊؋،؍؎؏ؘؙؚؐؑؒؓؔؕؖؗ؛؜؝؞؟ؠءآأؤإئابةتثجحخدذرزسشصضطظعغػؼؽؾؿـفقكلمنهوىيًٌٍَُِّْٕٖٜٟٓٔٗ٘ٙٚٛٝٞ٠";
          else if (sp == "4")
            s="123456788:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǦǧǨǩǪǫǬǭǮǯǰǴǵǶǷǸǹǼǽǾǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɌɍɐɑɒɓɔɕɖɗɘəɚɛɜɝɞɟɠɡɢɣɤɥɦɧɨɩɪɫɬɭɮɯɰɱɲɳɴɵɶɷɸɹɺɻɼɽɾɿʀʁʂʃʄʅʆʇʈʉʊʋʌʍʎʏʐʑʒʓʔʕʖʗʘʙʚʛʜʝʞʟʠʡʢʣʤʥʦʧʨʩʪʫʬʭʮʯʰʱʲʳʴʵʶʷʸʹʺʻʼʽʾʿˀˁ˂˃˄˅ˆˇˈˉˌˍˎˏːˑ˒˓˔˕˖˗˘˙˚˛˜˝˞˟ˠˡˢˣˤ˥˦˧˨˩˪˫ˬ˭ˮ˯˰˱˲˳˴˵˶˷˸˹˺˻˼˽˾˿̴̵̶̷̸̡̢̧̨̛̖̗̘̙̜̝̞̟̠̣̤̥̦̩̪̫̬̭̮̯̰̱̲̳̹̺̻̼͇͈͉͍͎̀́̂̃̄̅̆̇̈̉̊̋̌̍̎̏̐̑̒̓̔̽̾̿̀́͂̓̈́͆͊͋͌̕̚ͅͰͱͲͳʹ͵Ͷͷͺͻͼͽ;Ϳ΄΅Ά·ΈΉΊΌΎΏΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλμνξοπρςστυφχψωϊϋόύώϏϐϑϒϓϔϕϖϗϘϙϚϛϜϝϞϟϠϡϢϣϤϥϦϧϨϩϪϫϬϭϮϯϰϱϲϳϴϵ϶ϷϸϹϺϻϼϽϾϿЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюяѐёђѓєѕіїјљњћќѝўџѢѣѲѳҐґҒғҔҕҖҗҘҙҚқҢңҤҥҪҫҬҭҮүҰұҲҳӀӁӂӃӄӇӈӋӌӏӐӑӒӓӔӕӖӗӘәӚӛӜӝӞӟӠӡӢӣӤӥӦӧӨөӪӫӬӭӮӯӰӱӲӳӴӵӶӷӸӹԬԭԮԯ԰ԱԲԳԴԵԶԷԸԹԺԻԼԽԾԿՀՁՂՃՄՅՆՇՈՉՊՋՌՍՎՏՐՑՒՓՔՕՖՙ՚՛՜՝՞՟աբգդեզէըթժիլխծկհձղճմյնշոչպջռսվտրցւփքօֆև։֊֍֎֏־ֿ׀ׁׂ׃ׅׄ׆ׇ؀؁؂؃؄؅؆؇؈؉؊؋،؍؎؏ؘؙؚؐؑؒؓؔؕؖؗ؛؜؝؞؟ؠءآأؤإئابةتثجحخدذرزسشصضطظعغػؼؽؾؿـفقكلمنهوىيًٌٍَُِّْٕٖٜٟٓٔٗ٘ٙٚٛٝٞ٠";
          else
            s="toto12";
          w.resolve(seq, 0, '"' + to_htent(s) + '"');
        }).detach();
      },
      nullptr);

  w.bind_doc(
      "fstat", "gives information details on a file.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto sp=json_parse(req, "", 0);
          w.resolve(seq, 0, do_fstat(sp));
        }).detach();
      },
      nullptr);

  w.bind_doc("fwrite", "truncate and write to file with provided file name.",
             [&](const std::string &req) -> std::string {
               std::string fn, s;
               int n=0;
               fn=json_parse(req, "", n++);
               for (;;)
               {
                 s=json_parse(req, "", n++);
                 if (s.empty())
                   break;
                 if (n == 2)
                 {
                   fwrite(fn, s, std::ios::out);
                 }
                 else
                 {
                   fwrite(fn, "\n");
                   fwrite(fn, s);
                 }
               }
               return "";
             });

  w.bind_doc("fappend", "append to file with provided file name.", [&](const std::string &req) -> std::string {
    std::string fn, s;
    int n=0;
    fn=json_parse(req, "", n++);
    for (;;)
    {
      s=json_parse(req, "", n++);
      if (s.empty())
        break;
      if (n == 2)
      {
        fwrite(fn, s);
      }
      else
      {
        fwrite(fn, "\n");
        fwrite(fn, s);
      }
    }
    return "";
  });

  w.bind_doc("wait_nothread", "wait in foreground",
             [&](const std::string &req) -> std::string { return sec_wait(json_parse(req, "", 0)); });

  w.bind_doc(
      "wait_thread", "wait in background",
      [&](const std::string &seq, const std::string &req, void * /*arg*/) {
        std::thread([&, seq, req] {
          auto result=sec_wait(json_parse(req, "", 0));
          w.resolve(seq, 0, result);
        }).detach();
      },
      nullptr);

  w.bind_doc(
      "absolute", "gives absolute path of a file path.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto pth=json_parse(req, "", 0);
          auto res=std::filesystem::absolute(pth).string();
          replace_all(res, "\\", "/");
          w.resolve(seq, 0, '"' + res + '"');
        }).detach();
      },
      nullptr);

  w.bind_doc("chdir", "change current directory.", [&](const std::string &req) -> std::string {
    nfs=0;
    auto pth=json_parse(req, "", 0);
    std::filesystem::current_path(pth);
    return "";
  });

  w.bind_doc(
      "httpget", "run an http get with the provided url.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          std::string url=json_parse(req, "", 0);
          bool peer_check=s2b(json_parse(req, "", 1));
          bool host_check=s2b(json_parse(req, "", 2));
          bool verbose=s2b(json_parse(req, "", 3));
          std::cout << "URL " << url << ", peer_check " << peer_check << ", host_check " << host_check << ", verbose "
                    << verbose << std::endl;
          auto res=httpget(url, peer_check, host_check, verbose);
          std::cout << res << std::endl;
          w.resolve(seq, 0, res);
        }).detach();
      },
      nullptr);

  w.bind_doc("ls_attach", "list provided directory in foreground.", [&](const std::string &req) {
    auto param=json_parse(req, "", 0);
    auto res=lsdir(param);
    // std::cout << res << std::endl;
    return res;
  });

  w.bind_doc(
      "ls", "list provided directory in background",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto param=json_parse(req, "", 0);
          auto res=lsdir(param);
          // std::wcout << res << std::endl;
          w.resolve(seq, 0, res);
        }).detach();
      },
      nullptr);

  w.bind_doc(
      "lsr",
      "list provided directory recursively in background ==> very dangerous "
      "may hangs the machine !",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] { w.resolve(seq, 0, lsdir(json_parse(req, "", 0), true)); }).detach();
      },
      nullptr);

  w.bind_doc("webapp_get_title", "return webapp window title in foreground.",
             [&](const std::string &req) -> std::string {
               std::string prev_title="";
#ifdef _WIN32
               prev_title=GetWindowText((HWND)w.window());
#endif
               return "{\"value\": \"" + prev_title + "\"}";
             });

  w.bind_doc(
      "webapp_get_title_bg", "return webapp window title in background.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          std::string prev_title="";
#ifdef _WIN32
          prev_title=GetWindowText((HWND)w.window());
#endif
          auto result="{\"value\": \"" + prev_title + "\"}";
          w.resolve(seq, 0, result);
        }).detach();
      },
      nullptr);

  // Change window title
  w.bind_doc(
      "webapp_title", "change webapp window title.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          std::string prev_title="";
#ifdef _WIN32
          prev_title=GetWindowText((HWND)w.window());
#endif
          auto title=json_parse(req, "", 0);
          w.set_title(title);
          auto result="{\"value\": \"" + prev_title + "\"}";
          w.resolve(seq, 0, result);
        }).detach();
      },
      nullptr);

  // Change window dimension and sizing behaviour
  w.bind_doc("webapp_size", "set dimension and hint of webapp window.", [&](const std::string &req) -> std::string {
    auto params=json_parse(req, "", 0);
    auto l_width=std::stoi(json_parse(req, "", 0));
    auto l_height=std::stoi(json_parse(req, "", 1));
    auto l_hints=std::stoi(json_parse(req, "", 2));
    w.set_size(l_width, l_height, l_hints);
    return "";
  });

  // Exit from the web application
  w.bind_doc("webapp_exit", "exit from webapp.", [&](const std::string &) -> std::string {
    w.terminate();
    return "";
  });

  // Run a local command and return an eventual result at a later time.
  w.bind_doc(
      "webapp_exec", "run an external command.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto cmd=json_parse(req, "", 0);
          std::string res_cmd=exec_cmd(cmd);
          // std::cout << res_cmd << std::endl;
          std::string bs;
          bs=(char)92;
          replace_all(res_cmd, bs, "##BACKSLASH_CODE##");
          rep_crlf(res_cmd);
          replace_all(res_cmd, "##BACKSLASH_CODE##", bs + bs);
          auto result="{\"value\": \"" + res_cmd + "\"}";
          w.resolve(seq, 0, result);
        }).detach();
      },
      nullptr);

  w.bind_doc("write", "write a string to stdout.", [&](const std::string &req) -> std::string {
    auto s=json_parse(req, "", 0);
    write_cons(s);
    return "";
  });

  w.bind_doc("writeln", "write a string to stdout and add a carriage return.",
             [&](const std::string &req) -> std::string {
               auto s=json_parse(req, "", 0);
               write_cons(s);
               write_cons("\n");
               return "";
             });

  w.bind_doc("ewrite", "write a string to stderr.", [&](const std::string &req) -> std::string {
    auto s=json_parse(req, "", 0);
    write_cons(s, std::cerr);
    return "";
  });

  w.bind_doc("ewriteln", "write a string to stderr and add a carriage return.",
             [&](const std::string &req) -> std::string {
               auto s=json_parse(req, "", 0);
               write_cons(s, std::cerr);
               write_cons("\n", std::cerr);
               return "";
             });
}

bool run_and_exit=false;
void webview_set(bool devmode, int width, int height, int hints, bool _run_and_exit)
{
  void *wnd=nullptr;
  run_and_exit=_run_and_exit;

#ifdef _WIN32
  if (run_and_exit)
  {
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
      // Console mode, webview window will not be rendered.
      // HWND hwnd; extern HWND CreateWin(); hwnd=CreateWin(); wnd=&hwnd;
    }
    else
    {
      // GUI mode is not compatible with run_and_exit option
      run_and_exit=false;
      devmode=true;
    }
  }
#endif

  w.create(devmode, (void *)wnd);
  w.set_size(width, height, hints);
  create_binds(w);
}

void webview_run(std::string url, std::string title, std::string init_js)
{
  // std::cout << "URL " << url << ", TITLE " << title << ", JS " << init_js <<
  // std::endl;
  w.set_title(title);

  if (run_and_exit)
  {
    if (init_js.back() != ';')
      init_js += ';';
    init_js += " webapp_exit();";
    w.init(init_js);

    w.set_html("html://<div></div>");
  }
  else
  {
    if (!init_js.empty())
    {
      w.init(init_js);
    }

    if (url.starts_with("html://"))
    {
      w.set_html(url);
    }
    else
    {
      if (!url.starts_with("http://") && !url.starts_with("https://"))
      {
        url="file://" + url;
      }
      w.navigate(url);
    }
  }

  // std::cout << init_js << std::endl;
  w.run();
}

