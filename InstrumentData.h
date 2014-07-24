#ifdef Instrument_data_H
#define Instrument_data_H

class InstrumentData
{
public:
	InstrumentData();
	virtual ~InstrumentData();
	
	void Process(); 
	
	void populate(); // hot fix 
};
