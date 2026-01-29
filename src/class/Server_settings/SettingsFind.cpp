#include "SettingsServer.hpp"

#include <iostream>

#include "Tools1.hpp"

SettingsServer g_settings;

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a _global_settings (name value)
 *
 * @param set   Name of the setting to find
 * @return      Ptr to the string of the setting value, NULL otherwise		---*/
const std::string* SettingsServer::find_setting(const std::string& setting) const {

	map_strstr::const_iterator it = _global_settings.begin();
	it = _global_settings.find(setting);
	if (it == _global_settings.end()) {
		LOG_WARNING(C_511 "setting not found: " RESET << setting);
		return NULL;
	}
	else 
		return &(it->second);
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a global block (block_name "" {<block>})
 *
 * @param block_name   Name of the global block to find
 * @return      Pointer to const block, NULL if not found		---*/
const block* SettingsServer::find_global_block(const std::string& block_name) const {

	std::vector<block>::const_iterator it = _block_settings.begin();
	for ( ; it != _block_settings.end(); ++it) {
		if (it->name == block_name)
			return &(*it);
	}
	if (it == _block_settings.end()) {
		LOG_ERROR(RED "block not found: " RESET << block_name);
		return NULL;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for an arged block (block /path {<block>})
 *
 * @param block_name   Name of the blocks to find
 * @return      Vector of pointer to const block with said name		---*/
const std::vector<const block*> SettingsServer::find_arg_blocks(const std::string& block_name) const {

	std::vector<const block*> rtrn;
	for (std::vector<block>::const_iterator it = _block_settings.begin(); it != _block_settings.end(); ++it) {
		if (it->name == block_name && it->hasPath)
			rtrn.push_back(&(*it));
	}
	return rtrn;
}

//-----------------------------------------------------------------------------]
const block*	SettingsServer::find_arg_block_from_vector(const std::vector<const block*>& v, const std::string& arg_name) const {

	for (std::vector<const block*>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if ((*it)->path == arg_name)
			return *it;
	}
	return NULL;
}


//-----------------------------------------------------------------------------]
const std::string*	SettingsServer::find_setting_in_block(const block* b, const std::string& setting) const {

	if (!b)
		return NULL;
	map_strstr::const_iterator it = b->settings.find(setting);
	if (it == b->settings.end()) {
		LOG_DEBUG(C_522 "setting not found: " RESET << setting);
		return NULL;
	}
	else 
		return &it->second;
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a block setting
 *
 * @return const pointer to the value of setting if found, NULL otherwise			---*/
const std::string* SettingsServer::find_setting_in_blocks(const std::string& block_name, const std::string& arg, const std::string& setting) const {

	const block* b;
	if (arg == "")
		b = find_global_block(block_name);
	else {
		const std::vector<const block*> vect = find_arg_blocks(block_name);
		b = find_arg_block_from_vector(vect, arg);
	}
	return find_setting_in_block(b, setting);
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for root (location / {<block>})			---*/
block* SettingsServer::find_root_block() {

	std::vector<block>::iterator it = _block_settings.begin();
	for ( ; it != _block_settings.end(); ++it) {
		if (it->name == "location" && it->path == "/")
			return &(*it);
	}
	LOG_ERROR(RED "FATAL, Root location block / not found" RESET);
	return NULL;
}