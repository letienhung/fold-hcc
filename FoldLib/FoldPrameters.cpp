#include "FoldPrameters.h"


FoldPrameters::FoldPrameters()
{

}


FoldPrameters* FoldPrameters::getInstance()
{
	if (!_onlyInstance)
	{
		_onlyInstance = new FoldPrameters();
	}

	return _onlyInstance;
}
