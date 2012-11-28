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

	btr_sp modelSpace = GwaArx::Util::curDbBTR();
	xssert(modelSpace);

	AcDbObjectId id;
	ret_eOk(modelSpace->appendAcDbEntity(id, pPoint));
	xssert(id != AcDbObjectId::kNull);

	pPoint->close();

	return id;
}

AcDbObjectId GwaArx::Draw::aRectangle( 
  const ads_point corner1, 
  const ads_point corner2  
  )
{
	xssert(corner1 && corner2);

	SUPRESS_ORTHO_SNAP_OS_MODE_A_WHILE;

	ret_RTNORM(::acedCommand(
		RTSTR, TEXT("_.RECTANGLE"),
		RTPOINT, corner1,
		RTPOINT, corner2,
		RTNONE
		));
	
	ads_name name;	
	ret_RTNORM(::acdbEntLast(name));
	
	AcDbObjectId id;
	ret_eOk(::acdbGetObjectId(id, name));

	return id;
}

AcDbObjectId GwaArx::Draw::aPolyBox( 
	AcGePoint3dArray ptArr, 
	Adesk::Boolean closed /*= Adesk::kTrue */,
	bool bHighlight /*= true*/,
	Adesk::UInt16 color /*= 7*/,
	std::wstring layer /*= TEXT("0")*/)
{
	using namespace GwaArx::Util;

	db2dpline_sp pline = dbptr2sp<AcDb2dPolyline>(
		new AcDb2dPolyline(AcDb::k2dSimplePoly, ptArr, 0.0, closed));

	// set properties.
	pline->setColorIndex(color);
	pline->setLayer(TEXT("0"));

	// append to database
	AcDbObjectId id;
	ret_eOk(GwaArx::Util::curDbBTR()->appendAcDbEntity(id, pline.get()));
	xssert(id != AcDbObjectId::kNull);
	
	// highlight if required
	if (bHighlight)
	{
		pline.reset();
		id2sp<AcDb2dPolyline>(id)->highlight();
	}

	return id;
}
