#ifndef FORTIFY_CONSOLE
#define FORTIFY_CONSOLE

#include <mutex>
#include <deque>
#include <string>
#include <imgui.h>
#include "ResourceManager.h"

extern std::unique_ptr<ResourceManager> resources;

class Console {
public:
	ImGuiTextBuffer buffer;
	ImVector<int> lineOffsets;
	bool scrollToBottom = false;

	void clear() {
		buffer.clear();
		lineOffsets.clear();
	}

	//variadic function
	//IM_FMTARGS(2) => formatted string where second argument is the parameter
	//ex. add("%s", test)
	void add(const char* fmt, ...) IM_FMTARGS(2) {
		int oldSize = buffer.size();
		va_list args;
		va_start(args, fmt); // init argument list
		buffer.appendfv(fmt, args); // write str to buffer
		va_end(args);

		for (int newSize = buffer.size(); oldSize < newSize; oldSize++) {
			if (buffer[oldSize] == '\n') {
				lineOffsets.push_back(oldSize + 1);
			}
		}
		scrollToBottom = true;
	}

	void draw(const char* title, bool* open = nullptr) {
		if (!ImGui::Begin(title, open)) {
			ImGui::End();
			return;
		}
		
		if (ImGui::Button("Clear")) clear();
		ImGui::SameLine();
		if (ImGui::Button("Scroll to bottom")) scrollToBottom = true;
		ImGui::SameLine();
		if (ImGui::Button("Print resource log")) add("%s", resources->log().c_str());
		ImGui::SameLine();
		if (ImGui::Button("Copy")) ImGui::SetClipboardText(buffer.c_str());
		ImGui::Separator();

		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::TextUnformatted(buffer.begin());

		if (scrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		scrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};

struct LogBuffer {
	std::mutex mutex; //syncs msgs preventing concurrent r/w
	std::deque<std::string> messages;

	void push(const std::string& msg) {
		std::lock_guard<std::mutex> lock(mutex); // ensures exclusive access to msgs
		messages.push_back(msg);
	}

	void flush(Console& console) {
		std::lock_guard<std::mutex> lock(mutex);
		while (!messages.empty()) {
			console.add("%s", messages.front().c_str());
			messages.pop_front();
		}
	}

	void dumpToFile(const std::string& path) {
		std::ofstream file(path);
		if (!file.is_open()) return;

		std::lock_guard<std::mutex> lock(mutex);

		for (const auto& msg : messages) {
			file << msg << "\n";
		}
	}
};

#endif