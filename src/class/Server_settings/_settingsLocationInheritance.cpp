#include "Log.hpp"
#include "SettingsServer.hpp"

#include "Tools1.hpp"
#include <iostream>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////]
static std::vector<const block*>	rtrnMapOfMatches(const block& for_this_block);
static std::string rtrnWordMatch(const block& for_this_block, const std::string& setting_to_find, const std::vector<const block*>& map_of_matches);
static bool compareByPathLen(const block* a, const block* b);
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/** Initialize the internal data struct of all blocks that have a path.
 *
 * Invalid or unresolvable blocks are removed from _block_settings, with warnings logged.
 * @note Root '/' block is treated as already initialized.	---*/
void SettingsServer::setAllBlockLocations() {
	LOG_LOG("setAllBlockLocations()");

	for (std::vector<block>::iterator it = _block_settings.begin(); it != _block_settings.end(); ++it) {
		if (it->hasPath == false)
			continue;

		if (it->path[0] != '/') {
			LOG_ERROR("Location block without '/': " << it->path);
			it = _block_settings.erase(it);
			continue ;
		}

		if (it->name == "location" && it->path == "/")
			continue;
		
		std::vector<const block*> parent_tree = rtrnMapOfMatches(*it);
		if (!setLocationData(*it, parent_tree)) {
			LOG_WARNING(C_431 "Invalid location block detected, ignored:" RESET << *it);
			it = _block_settings.erase(it);
			continue;
		}
		LOG_DEBUG(C_431 "BLOCK: \n" RESET << *it);
	}
}

// #include <algorithm>
//-----------------------------------------------------------------------------]
/** Build an ordered list of ancestor location blocks for a given block.
 *
 * The returned vector is sorted from most specific (longest path) to least specific (/ root last).
 * Does not include the block itself.
 *
 * @param for_this_block  The block whose ancestors are being resolved
 * @return vector of ancestor blocks in descending specificity order	---*/
static std::vector<const block*>	rtrnMapOfMatches(const block& for_this_block) {

	const std::vector<const block *> locations = g_settings.find_arg_blocks("location");
	const std::string& given_path = for_this_block.path;
	std::vector<const block*>	v_rtrn;

	for (size_t i = 0; i < locations.size(); ++i) {

		const block* b = locations[i];
		const std::string& loc = b->path; // e.g. "/images"

		if (b == &for_this_block)
			continue;
		// must be a prefix
		if (given_path.compare(0, loc.size(), loc) != 0) // compare: given_path[0 .. loc.size()-1]  vs  loc
			continue;

		// boundary check: "/img" must not match "/images"
		if (loc != "/" && given_path.size() > loc.size() && given_path[loc.size()] != '/')
			continue;

		v_rtrn.push_back(b);
	}

	std::sort(v_rtrn.begin(), v_rtrn.end(), compareByPathLen);
	return v_rtrn;
}

///////////////////////////////////////////////////////////////////////////////]
/**  Populate a block's data fields using its ancestor locations.
 * Each setting is taken from the closest ancestor that defines it,
 * with the root location as a guaranteed fallback.
 * 
 * @return true if all settings were successfully set, false on error		
 * @note the root setting is checked further for validity and syntax/ ---*/
bool	SettingsServer::setLocationData(block& for_this_block, const std::vector<const block*>& map_of_matches) {

	std::string setting_match;

// std::string				root;
	for_this_block.data.root = rtrnWordMatch(for_this_block, "root", map_of_matches);
	if (!checkAnyRoot(for_this_block.data.root))
		return false;

// std::string				post_policy;
	for_this_block.data.post_policy = rtrnWordMatch(for_this_block, "post_policy", map_of_matches);

// std::string				cgi_interpreter;
	for_this_block.data.cgi_interpreter = rtrnWordMatch(for_this_block, "cgi_interpreter", map_of_matches);

// bool						autoindex;
	setting_match = rtrnWordMatch(for_this_block, "autoindex", map_of_matches);
	for_this_block.data.autoindex = (setting_match == "on");

// std::vector<std::string>	index;
	setting_match = rtrnWordMatch(for_this_block, "index", map_of_matches);
	std::string s;
	std::istringstream iss(setting_match);
	while (iss >> s) for_this_block.data.index.push_back(s);

// std::vector<std::string>	allowed_methods;
	setting_match = rtrnWordMatch(for_this_block, "allowed_methods", map_of_matches);
	s = "";
	iss.clear();
	iss.str(setting_match);
	while (iss >> s) for_this_block.data.allowed_methods.push_back(s);

// ssize_t					client_max_body_size;
	setting_match = rtrnWordMatch(for_this_block, "client_max_body_size", map_of_matches);
	if (!atoi_v2(setting_match, for_this_block.data.client_max_body_size))
		return false;

	return true;
}

//-----------------------------------------------------------------------------]
/** Return the first value of a setting found in the ancestor blocks, if not found first in the given block, or "" if none */
static std::string rtrnWordMatch(const block& for_this_block, const std::string& setting_to_find, const std::vector<const block*>& map_of_matches) {

	map_strstr::const_iterator match = for_this_block.settings.find(setting_to_find);
	if (match != for_this_block.settings.end()) //  && !match->second.empty() if i want inheritance in case of empty
		return match->second;

	for (std::vector<const block*>::const_iterator it = map_of_matches.begin(); it != map_of_matches.end(); ++it) {
		match = (*it)->settings.find(setting_to_find);
		if (match == (*it)->settings.end())
			continue ;
		return match->second;
	}
	return "";
}

//-----------------------------------------------------------------------------]
/**	Helper tool for sorting a std::vector<const block*>	---*/
static bool compareByPathLen(const block* a, const block* b) {
	return a->path.size() > b->path.size();
}