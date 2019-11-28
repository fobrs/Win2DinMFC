#pragma once


// Here are two global defines to use: 
struct ddraw_data_t
{
	ddraw_data_t();
	~ddraw_data_t();

	void WaitForVSync();
	void Initialize();

	DWORD GetFrequency();

};

