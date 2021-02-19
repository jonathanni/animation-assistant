/*
 * ahutil.h
 *
 *  Created on: Jul 23, 2019
 *      Author: Jonathan Ni
 */

#ifndef AHUTIL_H_
#define AHUTIL_H_

#include <fstream>
#include <ios>
#include <iostream>
#include <ctime>

#define AH_MEMORY_LIMIT (4LL*1024*1024*1024)
#define AH_BACK_MEMORY_SEC 2
#define AH_FWD_MEMORY_SEC 6
#define AH_DEFAULT_IMG_REFRESH_MS 100

wxDECLARE_EVENT(EVT_NULL, wxCommandEvent);

namespace ahutil {

const char *PNG_HEADER = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a";

bool isPNG(wxString filename) {
	std::ifstream in(filename.c_str(), std::ios::binary);
	char read[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	if (!in || !in.is_open() || in.eof())
		return false;

	in.read(read, 8);
	in.close();

	for (unsigned int i = 0; i < 8; i++)
		if (read[i] != PNG_HEADER[i])
			return false;

	return true;
}

// From https://stackoverflow.com/questions/800955/remove-if-equivalent-for-stdmap
// Author: IronSavior
template<typename ContainerT, typename PredicateT>
void erase_if(ContainerT &items, const PredicateT &predicate) {
	for (auto it = items.begin(); it != items.end();) {
		if (predicate(*it))
			it = items.erase(it);
		else
			++it;
	}
}

inline unsigned long long getImageSize(wxImage *img) {
	return img->GetSize().x * img->GetSize().y * 3LL * 8LL;
}

inline time_t getUniqueStamp() {
	return std::time(nullptr);
}

}

#endif /* AHUTIL_H_ */
