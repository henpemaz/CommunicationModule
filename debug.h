#ifndef _DEBUG_H_
#define _DEBUG_H_

/*
	Debug

	Some debug code that magically disappears at release !
	Define DB_MODULE before including the file to get the module name to show up

*/

#ifndef DB_MODULE
#define DB_MODULE "Debug"
#endif

#ifdef _DEBUG
#define db_start(...) do{Serial.begin(115200); while(!Serial);}while(0)
#define db_print(val) Serial.print(val)
#define db_println(val) Serial.println(val)

#define db_module(...) db_print(DB_MODULE) + db_print(" :")
#define db(val) db_module() + db_println(val)
#else
#define db_start(...)
#define db_print(val)
#define db_println(val)

#define db_module(...)
#define db(val)
#endif



#endif // !_DEBUG_H_
