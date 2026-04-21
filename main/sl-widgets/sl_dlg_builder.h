#ifndef SL_DLG_BUILDER_H_
#define SL_DLG_BUILDER_H_

#include <vector>
#include <Windows.h>


class CDialogueTemplate
{
public:
	CDialogueTemplate() = default;
	~CDialogueTemplate() = default;

	CDialogueTemplate(const CDialogueTemplate&) = delete;
	CDialogueTemplate& operator=(const CDialogueTemplate&) = delete;

	void SetWindowSize(unsigned short usWidth, unsigned short usHeight);
	void MakeWindowResizable(bool bResizable);
	void MakeWindowChild(bool bChild);


	const unsigned char* Generate(const wchar_t* wszWindowTitle = nullptr);

private:
	enum Constants { kBaseWidth = 200, kBaseHeight = 240 };

	WORD m_usWidth  = Constants::kBaseWidth;
	WORD m_usHeight = Constants::kBaseHeight;

	bool m_bResizable = false;
	bool m_bChild     = false;

	std::vector<unsigned char> m_buffer;
};

#endif
