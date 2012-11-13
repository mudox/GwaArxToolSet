#include "StdAfx.h"

#include "Draw.h"

#include "Util.h"

AcDbObjectId GwaArx::Draw::aPoint( const AcGePoint3d & pt )
{
	// set PDSIZE and PDMODE system variable.
	const short pdmode = 35;
	const short pdsize = -4;
	
	if(curDwg()->pdmode() != pdmode)
	{
		curDwg()->setPdmode(pdmode);
	}
	if (curDwg()->pdsize() != pdsize)
	{
		curDwg()->setPdsize(pdsize);
	}	

	// create & set point's attribute.
	AcDbPoint *pPoint = new AcDbPoint(pt);	
	pPoint->setLayer(TEXT("0"));

	btr_sp modelSpace = GwaArx::Util::openBlockTableRecord();
	xssert(modelSpace);

	AcDbObjectId id;
	ret_eOk(modelSpace->appendAcDbEntity(id, pPoint));
	xssert(id != AcDbObjectId::kNull);

	pPoint->close();

	return id;
}
