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
	std::vector<VkDebugUtilsMessageSeverityFlagBitsEXT> lineType;
	int selectionStart = -1;
	int selectionEnd = -1;
	bool isSelecting = false;

	struct InputTextColorData {
		ImU32(*line_color_func)(int line_index, const char* line_start, const char* line_end, void* user_data);
		void* user_data = nullptr;
	};

	static ImU32 colorCallback(int line, const char* start, const char* end, void* user_data) {
		const Console* console = static_cast<const Console*>(user_data);
		if (line >= 0 && line < static_cast<int>(console->lineType.size())) {
			switch (console->lineType[line]) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				return IM_COL32(255, 80, 80, 255);
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				return IM_COL32(255, 255, 100, 255);
			default:
				return IM_COL32(220, 220, 220, 255);
			}
		}

		return IM_COL32(220, 220, 220, 255);
	};

	void clear() {
		buffer.clear();
		lineOffsets.clear();
		lineType.clear();
	}

	//variadic function
	//IM_FMTARGS(2) => formatted string where second argument is the parameter
	//ex. add("%s", test)
	void add(VkDebugUtilsMessageSeverityFlagBitsEXT type, const char* fmt, ...) IM_FMTARGS(2) {
		int oldSize = buffer.size();
		va_list args;
		va_start(args, fmt); // init argument list
		buffer.appendfv(fmt, args); // write str to buffer
		va_end(args);

		for (int newSize = buffer.size(); oldSize < newSize; oldSize++) {
			if (buffer[oldSize] == '\n') {
				lineOffsets.push_back(oldSize + 1);
				lineType.push_back(type);
			}
		}
	}

	void draw(const char* title, bool* open = nullptr) {
		if (!ImGui::Begin(title, open, ImGuiWindowFlags_NoResize)) {
			ImGui::End();
			return;
		}
		
		if (ImGui::Button("Clear")) clear();
		ImGui::SameLine();
		if (ImGui::Button("Print resource log")) add(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "%s", resources->log().c_str());
		ImGui::SameLine();
		if (ImGui::Button("Copy")) ImGui::SetClipboardText(buffer.c_str());
		ImGui::Separator();

		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		const char* bufStart = buffer.begin();
		const char* bufEnd = buffer.end();
		const int lineCount = lineOffsets.Size;

		std::vector<char> displayBuffer;

		displayBuffer.assign(buffer.begin(), buffer.end());
		displayBuffer.push_back('\0');

		InputTextColorData colorData{};
		colorData.line_color_func = colorCallback;
		colorData.user_data = this;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::InputTextMultiline(
			"##console",
			displayBuffer.data(),
			displayBuffer.size(),
			ImGui::GetContentRegionAvail(),
			ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AllowTabInput,
			nullptr,
			&colorData
		);

		ImGui::PopStyleColor(3);

		ImGui::EndChild();
		ImGui::End();
	}
};

struct LogBuffer {
	std::mutex mutex; //syncs msgs preventing concurrent r/w
	std::deque<std::string> messages;
	std::deque<VkDebugUtilsMessageSeverityFlagBitsEXT> types;

	void push(const std::string& msg, VkDebugUtilsMessageSeverityFlagBitsEXT type) {
		std::lock_guard<std::mutex> lock(mutex); // ensures exclusive access to msgs
		messages.push_back(msg);
		types.push_back(type);
	}

	void flush(Console& console) {
		std::lock_guard<std::mutex> lock(mutex);
		while (!messages.empty()) {
			console.add(types.front(), "%s", messages.front().c_str());
			messages.pop_front();
			types.pop_front();
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