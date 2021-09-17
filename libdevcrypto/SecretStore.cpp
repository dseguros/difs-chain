#include "SecretStore.h"
#include <thread>
#include <mutex>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/Guards.h>
#include <libdevcore/SHA3.h>
#include <libdevcore/FileSystem.h>
#include <json_spirit/JsonSpiritHeaders.h>
#include <libdevcrypto/Exceptions.h>
using namespace std;
using namespace dev;
namespace js = json_spirit;
namespace fs = boost::filesystem;

static const int c_keyFileVersion = 3;

/// Upgrade the json-format to the current version.
static js::mValue upgraded(string const& _s)
{
	js::mValue v;
	js::read_string(_s, v);
	if (v.type() != js::obj_type)
		return js::mValue();
	js::mObject ret = v.get_obj();
	unsigned version = ret.count("Version") ? stoi(ret["Version"].get_str()) : ret.count("version") ? ret["version"].get_int() : 0;
	if (version == 1)
	{
		// upgrade to version 2
		js::mObject old;
		swap(old, ret);

		ret["id"] = old["Id"];
		js::mObject c;
		c["ciphertext"] = old["Crypto"].get_obj()["CipherText"];
		c["cipher"] = "aes-128-cbc";
		{
			js::mObject cp;
			cp["iv"] = old["Crypto"].get_obj()["IV"];
			c["cipherparams"] = cp;
		}
		c["kdf"] = old["Crypto"].get_obj()["KeyHeader"].get_obj()["Kdf"];
		{
			js::mObject kp;
			kp["salt"] = old["Crypto"].get_obj()["Salt"];
			for (auto const& i: old["Crypto"].get_obj()["KeyHeader"].get_obj()["KdfParams"].get_obj())
				if (i.first != "SaltLen")
					kp[boost::to_lower_copy(i.first)] = i.second;
			c["kdfparams"] = kp;
		}
		c["sillymac"] = old["Crypto"].get_obj()["MAC"];
		c["sillymacjson"] = _s;
		ret["crypto"] = c;
		version = 2;
	}
	if (ret.count("Crypto") && !ret.count("crypto"))
	{
		ret["crypto"] = ret["Crypto"];
		ret.erase("Crypto");
	}
	if (version == 2)
	{
		ret["crypto"].get_obj()["cipher"] = "aes-128-ctr";
		ret["crypto"].get_obj()["compat"] = "2";
		version = 3;
	}
	if (version == c_keyFileVersion)
		return ret;
	return js::mValue();
}
