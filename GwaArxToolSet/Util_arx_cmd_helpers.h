#pragma once

namespace GwaArx
{
	namespace Util
	{
		namespace _arx_cmd_helpers
		{
			typedef boost::function<void (void)> CmdImpFunc;

			/************************************
			// a outer-most try...catch structure wrapper. sinks exceptions and prints 
			// appropriate error message in command line window.
			// Note: the only one parameter is of a function pointer type "void (*)(void)"
			************************************/
			void InvokeCmdImp( CmdImpFunc imp );	

			/************************************
			// according the returning RTXXX value, throw appropriate exceptions.
			// encapsulate the common handling logic for common RTXXX return values.
			************************************/
			int rtCall( int rt );

			/************************************
			// according the returning Acad::ErrorStatus code value, 
			// throw appropriate exceptions.
			// encapsulate the common handling logic for common Acad::eXXX return values.
			************************************/
			void esCall( Acad::ErrorStatus err );

			// 1 byte empty exception class to indicate that user has canceled
			// (press ESC or Ctrl + C).
			struct UserCanceled
			{			
			};
		}
	}
}