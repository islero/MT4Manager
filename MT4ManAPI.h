#pragma once
#include <winsock.h>
#include <windows.h>
#using <mscorlib.dll>
#include <atlstr.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;
using namespace System::Collections;
using namespace System::IO;


namespace MetaQuotes {
#include "MT4ManagerAPI.h"
}
#include "Definitions.h"
namespace Manager {
		public delegate int ProgressFunc(int pos, int max, String ^Message);
		public delegate int UpdateFuncNotif(int login, String ^Group1, String ^Group2, String ^status1, String ^status2);

		//+------------------------------------------------------------------+
		//| Notification callback function                                   |
		//+------------------------------------------------------------------+
		[UnmanagedFunctionPointer(CallingConvention::StdCall)]
		public delegate void PumpingExDelegate(PumpingMessages code, TransTypes type, IntPtr data, IntPtr param);

		[UnmanagedFunctionPointer(CallingConvention::StdCall)]
		public delegate void PumpingDelegate(PumpingMessages code);
//+------------------------------------------------------------------+
//| Factory                                                          |
//+------------------------------------------------------------------+
		public ref class CManagerFactory {
		public:
			CManagerFactory()
			{
				WSADATA wsa;
				WSAStartup(0x0202, &wsa);

				char strPathName[_MAX_PATH];
				::GetModuleFileNameA(NULL, strPathName, _MAX_PATH);

				CStringA newPath(strPathName);
				int fpos = newPath.ReverseFind('\\');

				if (fpos != -1)
					newPath = newPath.Left(fpos + 1);

#ifndef _WIN64
				newPath += "mtmanapi.dll";
#else
				newPath += "mtmanapi64.dll";
#endif

				m_factory = new MetaQuotes::CManagerFactory(newPath.GetBuffer());
				m_factory->WinsockStartup();
			}

			CManagerFactory(char *path)
			{
				WSADATA wsa;
				WSAStartup(0x0202, &wsa);

				CStringA newPath(path);
				int fpos = newPath.ReverseFind('\\');

				if (fpos != -1)
					newPath = newPath.Left(fpos + 1);

#ifndef _WIN64
				newPath += "mtmanapi.dll";
#else
				newPath += "mtmanapi64.dll";
#endif

				m_factory = new MetaQuotes::CManagerFactory(newPath.GetBuffer());
				m_factory->WinsockStartup();
			}

			MetaQuotes::CManagerInterface *Create()
			{
				MetaQuotes::CManagerInterface *res;
				res = m_factory->Create(ManAPIVersion);
				return res;
			}

			MetaQuotes::CManagerFactory *m_factory;
		};


		public ref class MeTatraderManager : public IDisposable {
		private:
			MetaQuotes::CManagerInterface *m_manager;		
			GCHandle gch;
			//+------------------------------------------------------------------+
			//| Notification callback function                                   |
			//+------------------------------------------------------------------+
			delegate void MTAPI_NOTIFY_FUNC_EX_EventDelegate(int code, int type, void *data, void *param);
			delegate void MTAPI_NOTIFY_FUNC_EX_EventDelegateCS(int code, int type);
			

		public:

			MeTatraderManager()
			{
				CManagerFactory fact;
				m_manager = fact.Create();
			}
			MeTatraderManager(String ^dir)
			{
				char *_dir = (char *)(void *)Marshal::StringToHGlobalAnsi(dir);
				CManagerFactory fact(_dir);
				m_manager = fact.Create();
				m_manager->WorkingDirectory(_dir);
				Marshal::FreeHGlobal(IntPtr((void *)_dir));

			}
			virtual ~MeTatraderManager()
			{
				try {
					if (gch.IsAllocated) {
						gch.Free();
					}
					if (m_manager->IsConnected())
						m_manager->Disconnect();
					(RetValues)m_manager->Release();
				}
				catch (...) {
					;
				}
			}

			bool IsValid()
			{
				return m_manager != NULL;
			}

			//--- service methods
			String ^ErrorDescription(RetValues code)
			{
				const int icode = (int)code;
				char *text = (char *)m_manager->ErrorDescription(icode);
				return Marshal::PtrToStringAnsi(IntPtr(text));
			}

			//--- connection
			RetValues Connect(String ^server)
			{
				char *_server = (char *)(void *)Marshal::StringToHGlobalAnsi(server);
				int res = m_manager->Connect(_server);
				Marshal::FreeHGlobal(IntPtr((void *)_server));
				return (RetValues)res;
			}

			RetValues Disconnect()
			{
				if (gch.IsAllocated) {
					gch.Free();
				}
				return (RetValues)m_manager->Disconnect();
			}

			bool IsConnected()
			{
				return m_manager->IsConnected() > 0;
			}

			RetValues Login(const int login, String ^password)
			{
				char *_password = (char *)(void *)Marshal::StringToHGlobalAnsi(password);
				int res = m_manager->Login(login, _password);
				Marshal::FreeHGlobal(IntPtr((void *)_password));
				return (RetValues)res;
			}

			RetValues LoginSecured(String ^key_path)
			{
				char *_key_path = (char *)(void *)Marshal::StringToHGlobalAnsi(key_path);
				int res = m_manager->LoginSecured(_key_path);
				Marshal::FreeHGlobal(IntPtr((void *)_key_path));
				return (RetValues)res;
			}

			RetValues KeysSend(String ^key_path)
			{
				char *_key_path = (char *)(void *)Marshal::StringToHGlobalAnsi(key_path);
				int res = m_manager->KeysSend(_key_path);
				Marshal::FreeHGlobal(IntPtr((void *)_key_path));
				return (RetValues)res;
			}

			RetValues Ping()
			{
				return (RetValues)m_manager->Ping();
			}	

			RetValues PasswordChange(String ^pass, const int is_investor)
			{
				char *_password = (char *)(void *)Marshal::StringToHGlobalAnsi(pass);
				int res = m_manager->PasswordChange(_password, is_investor);
				Marshal::FreeHGlobal(IntPtr((void *)_password));
				return (RetValues)res;
			}
			RetValues ManagerRights([Out] ConManager %man)
			{
				MetaQuotes::ConManager mngr = { 0 };
				int res = m_manager->ManagerRights(&mngr);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				man = ConManager(mngr);
				return (RetValues)res;

			}

			//--- server administration commands
			RetValues SrvRestart()
			{
				return (RetValues)m_manager->SrvRestart();
			}

			RetValues SrvChartsSync()
			{
				return (RetValues)m_manager->SrvChartsSync();
			}

			RetValues SrvLiveUpdateStart()
			{
				return (RetValues)m_manager->SrvLiveUpdateStart();
			}

			RetValues SrvFeedsRestart()
			{
				return (RetValues)m_manager->SrvFeedsRestart();
			}


			//--- server configuration
			//--- configuration request
			RetValues CfgRequestCommon([Out]ConCommon %cfg)
			{
				MetaQuotes::ConCommon conf = { 0 };
				int res = m_manager->CfgRequestCommon(&conf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				cfg = ConCommon(conf);
				return (RetValues)res;

			}

			RetValues CfgRequestTime([Out]ConTime %cfg)
			{
				MetaQuotes::ConTime conf = { 0 };
				int res = m_manager->CfgRequestTime(&conf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				cfg = ConTime(conf);
				return (RetValues)res;

			}
			RetValues CfgRequestBackup([Out] ConBackup %cfg)
			{
				MetaQuotes::ConBackup conf = { 0 };
				int res = m_manager->CfgRequestBackup(&conf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				cfg = ConBackup(conf);
				return (RetValues)res;

			}
			RetValues CfgRequestSymbolGroup([Out] ConSymbolGroup %cfg)
			{
				MetaQuotes::ConSymbolGroup conf = { 0 };
				int res = m_manager->CfgRequestSymbolGroup(&conf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				cfg = ConSymbolGroup(conf);
				return (RetValues)res;

			}


			List<ConAccess>^ CfgRequestAccess()
			{
				List<ConAccess> ^res = gcnew List<ConAccess>();
				int total = 0;
				MetaQuotes::ConAccess *cs = NULL;
				cs = m_manager->CfgRequestAccess(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConAccess(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConDataServer>^ CfgRequestDataServer()
			{
				List<ConDataServer> ^res = gcnew List<ConDataServer>();
				int total = 0;
				MetaQuotes::ConDataServer *cs = NULL;
				cs = m_manager->CfgRequestDataServer(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConDataServer(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConHoliday>^ CfgRequestHoliday()
			{
				List<ConHoliday> ^res = gcnew List<ConHoliday>();
				int total = 0;
				MetaQuotes::ConHoliday *cs = NULL;
				cs = m_manager->CfgRequestHoliday(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConHoliday(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}

			List<ConSymbol>^ CfgRequestSymbol()
			{
				List<ConSymbol> ^res = gcnew List<ConSymbol>();
				int total = 0;
				MetaQuotes::ConSymbol *cs = NULL;
				cs = m_manager->CfgRequestSymbol(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConSymbol(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
	
			List<ConGroup^>^ CfgRequestGroup()
			{
				List<ConGroup^> ^res = gcnew List<ConGroup^>();
				int total = 0;
				MetaQuotes::ConGroup *cs = NULL;
				cs = m_manager->CfgRequestGroup(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(gcnew ConGroup(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConManager>^ CfgRequestManager()
			{
				List<ConManager> ^res = gcnew List<ConManager>();
				int total = 0;
				MetaQuotes::ConManager *cs = NULL;
				cs = m_manager->CfgRequestManager(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConManager(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConFeeder>^ CfgRequestFeeder()
			{
				List<ConFeeder> ^res = gcnew List<ConFeeder>();
				int total = 0;
				MetaQuotes::ConFeeder *cs = NULL;
				cs = m_manager->CfgRequestFeeder(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConFeeder(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConLiveUpdate>^ CfgRequestLiveUpdate()
			{
				List<ConLiveUpdate> ^res = gcnew List<ConLiveUpdate>();
				int total = 0;
				MetaQuotes::ConLiveUpdate *cs = NULL;
				cs = m_manager->CfgRequestLiveUpdate(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConLiveUpdate(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConSync>^ CfgRequestSync()
			{
				List<ConSync> ^res = gcnew List<ConSync>();
				int total = 0;
				MetaQuotes::ConSync *cs = NULL;
				cs = m_manager->CfgRequestSync(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConSync(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			List<ConPluginParam>^ CfgRequestPlugin()
			{
				List<ConPluginParam> ^res = gcnew List<ConPluginParam>();
				int total = 0;
				MetaQuotes::ConPluginParam *cs = NULL;
				cs = m_manager->CfgRequestPlugin(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConPluginParam(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}

			//--- configuration update
			RetValues CfgUpdateCommon(ConCommon^ cfg)
			{
				MetaQuotes::ConCommon conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues) m_manager->CfgUpdateCommon(&conf);
				

			}
			RetValues CfgUpdateAccess(ConAccess^ cfg, const int pos)
			{
				MetaQuotes::ConAccess conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateAccess(&conf,pos);
				

			}
			RetValues CfgUpdateDataServer(ConDataServer^ cfg, const int pos)
			{
				MetaQuotes::ConDataServer conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateDataServer(&conf, pos);
				

			}

			RetValues CfgUpdateTime(ConTime^ cfg)
			{
				MetaQuotes::ConTime conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateTime(&conf);
				

			}

			RetValues CfgUpdateHoliday(ConHoliday^ cfg, const int pos)
			{
				MetaQuotes::ConHoliday conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateHoliday(&conf, pos);

			}
			
			RetValues CfgUpdateSymbol(ConSymbol^ cfg)
			{
				MetaQuotes::ConSymbol conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues) m_manager->CfgUpdateSymbol(&conf);
				
			}

			RetValues CfgUpdateSymbolGroup(ConSymbolGroup^ cfg, const int pos)
			{
				MetaQuotes::ConSymbolGroup conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateSymbolGroup(&conf, pos);
			}

			RetValues CfgUpdateGroup(ConGroup^ cfg)
			{
				MetaQuotes::ConGroup conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateGroup(&conf);
				
			}

			RetValues CfgUpdateManager(ConManager^ cfg)
			{
				MetaQuotes::ConManager conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues) m_manager->CfgUpdateManager(&conf);
				
			}

			RetValues CfgUpdateFeeder(ConFeeder^ cfg)
			{
				MetaQuotes::ConFeeder conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateFeeder(&conf);
				
			}

			RetValues CfgUpdateBackup(ConBackup^ cfg)
			{
				MetaQuotes::ConBackup conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues)m_manager->CfgUpdateBackup(&conf);
				
			}

			RetValues CfgUpdateLiveUpdate(ConLiveUpdate^ cfg)
			{
				MetaQuotes::ConLiveUpdate conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues) m_manager->CfgUpdateLiveUpdate(&conf);
			
			}
			
			RetValues CfgUpdateSync(ConSync^ cfg)
			{
				MetaQuotes::ConSync conf = { 0 };
				cfg->ToMT4(&conf);
				return (RetValues) m_manager->CfgUpdateSync(&conf);
	
			}

			RetValues CfgUpdatePlugin(ConPlugin^ cfg, List<PluginCfg^>^ parupd) {
			
				MetaQuotes::ConPlugin *conf = NULL;
				
				cfg->ToMT4(conf);
				

				MetaQuotes::PluginCfg *plg = NULL;

				for (int i = 0; i < parupd->Count; i++)
				{
					parupd[i]->ToMT4(&plg[i]);
				}


				int res=m_manager->CfgUpdatePlugin(conf, plg, parupd->Count);

				delete[] plg;
				delete conf;
				return  (RetValues)res;
			
			}
			
			//--- configuration delete
			RetValues CfgDeleteAccess(int pos)
			{
				return (RetValues)m_manager->CfgDeleteAccess(pos);
			}

			RetValues CfgDeleteDataServer(int pos)
			{
				return (RetValues)m_manager->CfgDeleteDataServer(pos);
			}

			RetValues CfgDeleteHoliday(int pos)
			{
				return (RetValues)m_manager->CfgDeleteHoliday(pos);
			}

			RetValues CfgDeleteSymbol(int pos)
			{
				return (RetValues)m_manager->CfgDeleteSymbol(pos);
			}

			RetValues CfgDeleteGroup(int pos)
			{
				return (RetValues)m_manager->CfgDeleteGroup(pos);
			}

			RetValues CfgDeleteManager(int pos)
			{
				return (RetValues)m_manager->CfgDeleteManager(pos);
			}

			RetValues CfgDeleteFeeder(int pos)
			{
				return (RetValues)m_manager->CfgDeleteFeeder(pos);
			}

			RetValues CfgDeleteLiveUpdate(int pos)
			{
				return (RetValues)m_manager->CfgDeleteLiveUpdate(pos);
			}

			RetValues CfgDeleteSync(const int pos)
			{
				return (RetValues)m_manager->CfgDeleteSync(pos);
			}

			//--- configuration shift
			RetValues CfgShiftAccess(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftAccess(pos,shift);
			}
			RetValues CfgShiftDataServer(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftDataServer(pos, shift);
			}
			RetValues CfgShiftHoliday(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftHoliday(pos, shift);
			}
			RetValues CfgShiftSymbol(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftSymbol(pos, shift);
			}
			RetValues CfgShiftGroup(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftGroup(pos, shift);
			}
			RetValues CfgShiftManager(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftManager(pos, shift);
			}
			RetValues CfgShiftFeeder(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftFeeder(pos, shift);
			}
			RetValues CfgShiftLiveUpdate(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftLiveUpdate(pos, shift);
			}
			RetValues CfgShiftSync(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftSync(pos, shift);
			}
			RetValues CfgShiftPlugin(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftPlugin(pos, shift);
			}

			//--- server feeders
			List<ServerFeed>^ SrvFeeders()
			{
				List<ServerFeed> ^res = gcnew List<ServerFeed>();
				int total = 0;
				MetaQuotes::ServerFeed *cs = NULL;
				cs = m_manager->SrvFeeders(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ServerFeed(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}

			String^   SrvFeederLog(String^ name) {
				int len;
				char *_name = (char *)(void *)Marshal::StringToHGlobalAnsi(name);
				char *text = (char *)m_manager->SrvFeederLog(_name, &len);
				Marshal::FreeHGlobal(IntPtr((void *)_name));
				return gcnew String(text);
			}

			//--- performance info
			List<PerformanceInfo>^ PerformanceRequest(UInt32 from)
			{
				List<PerformanceInfo> ^res = gcnew List<PerformanceInfo>();
				int total = 0;
				MetaQuotes::PerformanceInfo *cs = NULL;
				cs = m_manager->PerformanceRequest(from,&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(PerformanceInfo(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			
			//--- users/trades backups
			List<BackupInfo>^ BackupInfoUsers( int mode)
			{
				List<BackupInfo> ^res = gcnew List<BackupInfo>();
				int total = 0;
				MetaQuotes::BackupInfo *bkp = NULL;
				bkp = m_manager->BackupInfoUsers(mode, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(BackupInfo(bkp[i]));
				}
				m_manager->MemFree(bkp);
				return res;
			}

			List<BackupInfo>^ BackupInfoOrders( int mode)
			{
				List<BackupInfo> ^res = gcnew List<BackupInfo>();
				int total = 0;
				MetaQuotes::BackupInfo *cs = NULL;
				cs = m_manager->BackupInfoOrders(mode, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(BackupInfo(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}

			List<UserRecord>^ BackupRequestUsers(String^ file, String^ request)
			{
				List<UserRecord> ^res = gcnew List<UserRecord>();
				int total = 0;
				MetaQuotes::UserRecord *cs = NULL;
				char* _file=(char *)(void *)Marshal::StringToHGlobalAnsi(file);
				char* _request = (char *)(void *)Marshal::StringToHGlobalAnsi(request);
				cs = m_manager->BackupRequestUsers(_file, _request, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(UserRecord(cs[i]));
				}
				m_manager->MemFree(cs);
				Marshal::FreeHGlobal(IntPtr((void *)_file));
				Marshal::FreeHGlobal(IntPtr((void *)_request));
				return res;
			}


			List<TradeRecord>^ BackupRequestOrders(String^ file, String^ request)
			{
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *cs = NULL;
				char* _file = (char *)(void *)Marshal::StringToHGlobalAnsi(file);
				char* _request = (char *)(void *)Marshal::StringToHGlobalAnsi(request);
				cs = m_manager->BackupRequestOrders(_file, _request, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(cs[i]));
				}
				m_manager->MemFree(cs);
				Marshal::FreeHGlobal(IntPtr((void *)_file));
				Marshal::FreeHGlobal(IntPtr((void *)_request));
				return res;
			}
			
			RetValues BackupRestoreUsers(List<UserRecord> ^users)
			{
	
				MetaQuotes::UserRecord *usr = NULL;
				for (int i = 0; i < users->Count; i++)
				{
					users[i].ToMT4(&usr[i]);
				}
				int res=m_manager->BackupRestoreUsers(usr, users->Count);
				delete usr;
				return (RetValues)res;
				
			}

			
			List<TradeRestoreResult>^ BackupRestoreOrders(List<TradeRecord> ^trades)
			{
				MetaQuotes::TradeRecord *trds = NULL;

				for (int i = 0; i < trades->Count; i++)
				{
					trades[i].ToMT4(&trds[i]);
				}

				List<TradeRestoreResult> ^res = gcnew List<TradeRestoreResult>();
				int total = 0;
				MetaQuotes::TradeRestoreResult *trs = NULL;
				trs = m_manager->BackupRestoreOrders(trds, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRestoreResult(trs[i]));
				}
				m_manager->MemFree(trs);
				delete trds;
				return res;
			}

			//--- administrator databases commands
			List<UserRecord>^ AdmUsersRequest(String^ group)
			{
				List<UserRecord> ^res = gcnew List<UserRecord>();
				int total = 0;
				MetaQuotes::UserRecord *cs = NULL;
				char * _group=(char *)(void *)Marshal::StringToHGlobalAnsi(group);
				cs = m_manager->AdmUsersRequest(_group, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(UserRecord(cs[i]));
				}
				m_manager->MemFree(cs);
				Marshal::FreeHGlobal(IntPtr((void *)_group));
				return res;
			}

			List<TradeRecord>^ AdmTradesRequest(String^ group, int open_only)
			{
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *cs = NULL;
				char * _group = (char *)(void *)Marshal::StringToHGlobalAnsi(group);
				cs = m_manager->AdmTradesRequest(_group, open_only, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(cs[i]));
				}
				m_manager->MemFree(cs);
				Marshal::FreeHGlobal(IntPtr((void *)_group));
				return res;
			}

			RetValues AdmBalanceCheckObsolete(List<int>^ logins)
			{
				int total = logins->Count;
				int* usr=new int[total] ;
				

				for (int i = 0; i < logins->Count; i++)
				{
					usr[i] = logins[i];
				}

				RetValues res=(RetValues)m_manager->AdmBalanceCheckObsolete(usr, &total);
				delete[]usr;
				return res;

			}

			RetValues AdmBalanceFix(List<int>^ logins)
			{
				int total = logins->Count;
				int* usr = new int[total];
				for (int i = 0; i < total; i++)
					usr[i] = logins[i];

				RetValues res = (RetValues)m_manager->AdmBalanceFix(usr, total);

				delete[] usr;

				return res;
			}


			RetValues AdmTradesDelete(List<int>^ orders)
			{
				int total = orders->Count;
				int* ordrs = new int[total];
				for (int i = 0; i < total; i++)
					ordrs[i] = orders[i];

				RetValues res = (RetValues)m_manager->AdmTradesDelete(ordrs, total);

				delete[] ordrs;

				return res;
			}

			RetValues AdmTradeRecordModify(List<TradeRecord> ^trade)
			{

				MetaQuotes::TradeRecord *trd = NULL;

				for (int i = 0; i < trade->Count; i++)
				{
					trade[i].ToMT4(&trd[i]);
				}

				int res=m_manager->AdmTradeRecordModify(trd);
				delete trd;
				return (RetValues)res;

			}

			//--- symbols
			int SymbolsRefresh()
			{
				return  m_manager->SymbolsRefresh();

			}
		
			List<ConSymbol> ^SymbolsGetAll()
			{
				List<ConSymbol> ^res = gcnew List<ConSymbol>();
				int total = 0;
				MetaQuotes::ConSymbol *cs = NULL;
				cs = m_manager->SymbolsGetAll(&total);
				for (int i = 0; i < total; i++) {
					res->Add(ConSymbol(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}
			
			RetValues SymbolGet(String ^symbol, [Out] ConSymbol %conSym)
			{
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);
				MetaQuotes::ConSymbol cs = { 0 };

				int r = m_manager->SymbolGet(_symbol, &cs);
				if (r == 0)
					conSym = ConSymbol(cs);


				Marshal::FreeHGlobal(IntPtr((void *)_symbol));

				return (RetValues)r;
			}

			RetValues SymbolInfoGet(String^ basec, [Out]SymbolInfo % symbIn)
			{

				MetaQuotes::SymbolInfo si;
				char *_basec = (char *)(void *)Marshal::StringToHGlobalAnsi(basec);
				int res = m_manager->SymbolInfoGet(_basec, &si);

				if (res == MetaQuotes::RET_OK) symbIn = SymbolInfo(si);
				Marshal::FreeHGlobal(IntPtr((void *)_basec));
				return (RetValues)res;
			}
		
			RetValues SymbolAdd(String^ symbol)
			{
				char* _symbol = (char*)(void*)Marshal::StringToHGlobalAnsi(symbol);
				int res = m_manager->SymbolAdd(_symbol);
				
				Marshal::FreeHGlobal(IntPtr((void*)_symbol));
				return (RetValues)res;
			}

			RetValues SymbolHide(String^ symbol)
			{
				char* _symbol = (char*)(void*)Marshal::StringToHGlobalAnsi(symbol);
				int res = m_manager->SymbolHide(_symbol);

				Marshal::FreeHGlobal(IntPtr((void*)_symbol));
				return (RetValues)res;
			}

			RetValues SymbolSendTick(String^ symbol, double bid,  double ask)
			{
				char* _symbol = (char*)(void*)Marshal::StringToHGlobalAnsi(symbol);
				int res = m_manager->SymbolSendTick(_symbol,bid,ask);

				Marshal::FreeHGlobal(IntPtr((void*)_symbol));
				return (RetValues)res;

			}

			//--- manager commands
			List<ConGroup^> ^GroupsRequest()
			{
				List<ConGroup^> ^res = gcnew List<ConGroup^>();
				int total = 0;
				MetaQuotes::ConGroup *cs = NULL;
				cs = m_manager->GroupsRequest(&total);
				for (int i = 0; i < total; i++) {
					res->Add(gcnew ConGroup(cs[i]));
				}
				m_manager->MemFree(cs);
				return res;
			}

			RetValues MailSend(MailBox mail,List<int>^ logins) {
			
				int total = logins->Count;
				int* usr = new int[total];
				for (int i = 0; i < total; i++)
					usr[i] = logins[i];
				MetaQuotes::MailBox mails;
				mail.ToMT4(&mails);
				int res = m_manager->MailSend(&mails, usr);
				delete[]usr;
				return (RetValues)res;

			
			}

			RetValues NewsSend(NewsTopic news) {
				MetaQuotes::NewsTopic nt;
				news.ToMT4(&nt);
				int res = m_manager->NewsSend(&nt);
				return (RetValues)res;
			}

			//--- journal
			List<ServerLog> ^JournalRequest(EnLogType mode, UInt32 from, UInt32 to, String ^filter)
			{
				List<ServerLog> ^res = gcnew List<ServerLog>();
				int total = 0;
				char *_filter = (char *)(void *)Marshal::StringToHGlobalAnsi(filter);
				MetaQuotes::ServerLog *sl = NULL;			
				sl = m_manager->JournalRequest((int)mode, from, to, _filter, &total);
				for (int i = 0; i < total; i++) {
					res->Add(ServerLog(sl[i]));
				}

				m_manager->MemFree(sl);
				Marshal::FreeHGlobal(IntPtr((void *)_filter));
				return res;
			}

			//--- databases: direct request to the server
			//--- users
			List<UserRecord>^ UsersRequest()
			{
				List<UserRecord>^ result = gcnew List<UserRecord>();
				int total = 0;
				MetaQuotes::UserRecord* usrs = m_manager->UsersRequest(&total);
				if (total > 0)
					result->Capacity = total;
				for (int i = 0; i < total; i++)
				{
					UserRecord ur = UserRecord(usrs[i]);
					result->Add(ur);
				}
				if (total > 0)
					m_manager->MemFree(usrs);
				return result;
			}


			List<UserRecord>^ UserRecordsRequest(List<int>^ logins)
			{
			int total = logins->Count;
			int* usr = new int[total];
			for (int i = 0; i < total; i++)
				usr[i] = logins[i];

			
			MetaQuotes::UserRecord *usrs = m_manager->UserRecordsRequest(usr, &total);
			List<UserRecord>^ result = gcnew List<UserRecord>();
			for (int i = 0; i < total; i++)
			{
				UserRecord ur = UserRecord(usrs[i]);
				result->Add(ur);
			}
		
			if (total > 0)
				m_manager->MemFree(usrs);
			delete[] usr;
			return result;
			}

			RetValues UserRecordUpdate([Out] UserRecord %UserRec)
			{

				MetaQuotes::UserRecord UserInfo;
				UserRec.ToMT4(&UserInfo);
				int res = m_manager->UserRecordUpdate(&UserInfo);

				if (res == MetaQuotes::RET_OK)
					UserRec = UserRecord(UserInfo);

				return (RetValues)res;
			}


			RetValues UserRecordNew([Out] UserRecord %UserRec)
			{

				MetaQuotes::UserRecord UserInfo;
				UserRec.ToMT4(&UserInfo);
				int res = m_manager->UserRecordNew(&UserInfo);

				if (res == MetaQuotes::RET_OK)
					UserRec = UserRecord(UserInfo);

				return (RetValues)res;
			}

			RetValues UsersGroupOp(List<GroupCommandInfo>^ info, List<int>^ logins)
			{

				MetaQuotes::GroupCommandInfo *inf = NULL;

				for (int i = 0; i < info->Count; i++)
				{
					info[i].ToMT4(&inf[i]);
				}

				int* logs = new int[logins->Count];
				for (int i = 0; i < logins->Count; i++)
					logs[i] = logins[i];

				RetValues res = (RetValues)m_manager->UsersGroupOp(inf, logs);

				delete inf;
				delete[] logs;

				return res;
			
			}



			RetValues UserPasswordCheck(int login,String^ password)
			{
				char* pass = (char*)(void*)Marshal::StringToHGlobalAnsi(password);
				int res = m_manager->UserPasswordCheck(login,pass);

				Marshal::FreeHGlobal(IntPtr((void*)pass));
				return (RetValues)res;
			}

			RetValues UserPasswordSet(int login, String ^password, int change_investor)
			{

				char *pass = (char *)(void *)Marshal::StringToHGlobalAnsi(password);

				int res = m_manager->UserPasswordSet(login, pass, change_investor, 0);
				Marshal::FreeHGlobal(IntPtr((void *)pass));

				return (RetValues)res;
			}

			List<OnlineRecord>^ OnlineRequest()
			{
				MetaQuotes::OnlineRecord* online;
				int total;
				online = m_manager->OnlineRequest(&total);
				List<OnlineRecord>^ OnlineRecords = gcnew List<OnlineRecord>();
				for (int i = 0; i < total; i++)
				{
					OnlineRecord records = OnlineRecord(online[i]);
					OnlineRecords->Add(records);
				}
				if(total>0)
				m_manager->MemFree(online);
				return OnlineRecords;
			}

			//--- orders
			RetValues TradeTransaction(TradeTransInfo^ tti, [Out]Int32 % order)
			{
				MetaQuotes::TradeTransInfo tti_api = { 0 };
				tti->ToMT4(&tti_api);
				int res = m_manager->TradeTransaction(&tti_api);
				if (res == MetaQuotes::RET_OK) order = tti_api.order;
				return (RetValues)res;
			}

			List<TradeRecord> ^TradesRequest()
			{

				List<TradeRecord> ^result = gcnew List<TradeRecord>();
				int total = 0;

				MetaQuotes::TradeRecord *tr = m_manager->TradesRequest(&total);

				for (int i = 0; i < total; i++) {
					result->Add(TradeRecord(tr[i]));
				}
				m_manager->MemFree(tr);
				return result;

			}

			List<TradeRecord>^ TradeRecordsRequest(List<int>^ orders)
			{
				List<TradeRecord> ^result = gcnew List<TradeRecord>();
				int total = orders->Count;
				int* ordrs = new int[total];


				for (int i = 0; i < orders->Count; i++)
				{
					ordrs[i] = orders[i];
				}
				MetaQuotes::TradeRecord *trs = m_manager->TradeRecordsRequest(ordrs, &total);

				for (int i = 0; i < total; i++) {
					result->Add(TradeRecord(trs[i]));
				}
				m_manager->MemFree(trs);
				delete[] ordrs;
				return result;
			}

			List<TradeRecord> ^TradesUserHistory(int login, int from, int to)
			{

				List<TradeRecord> ^result = gcnew List<TradeRecord>();
				int total = 0;

				MetaQuotes::TradeRecord *tr = m_manager->TradesUserHistory(login, from, to, &total);
				for (int i = 0; i < total; i++) {
					result->Add(TradeRecord(tr[i]));
				}
				m_manager->MemFree(tr);
				return result;

			}
			RetValues TradeCheckStops(List<TradeTransInfo> ^ trade,  double price) {
				MetaQuotes::TradeTransInfo *inf = NULL;

				for (int i = 0; i < trade->Count; i++)
				{
					trade[i].ToMT4(&inf[i]);
				}

				RetValues res=(RetValues)m_manager->TradeCheckStops(inf, price);
				delete inf;
				return res;
			
			}

			//--- reports
			List<TradeRecord>^ ReportsRequest(ReportGroupRequest^ request, List<int>^ logins)
			{

				
				MetaQuotes::ReportGroupRequest reqsts;
				
				request->ToMT4(&reqsts);
				
				int total = 0;

				int* logs = new int[logins->Count];
				for (int i = 0; i < logins->Count; i++)
					logs[i] = logins[i];

				
				MetaQuotes::TradeRecord* records = m_manager->ReportsRequest(&reqsts, logs, &total);
				List<TradeRecord>^ result = gcnew List<TradeRecord>();
				for (int i = 0; i < total; i++) {
					result->Add(TradeRecord(records[i]));
				}

				m_manager->MemFree(records);
				
				delete[] logs;

				return result;
			}

			List<DailyReport>^ DailyReportsRequest(List<DailyGroupRequest>^ request, List<int>^ logins)
			{

				int total = 0;
				MetaQuotes::DailyGroupRequest* reqsts = NULL;
				reqsts->total = logins->Count;

				for (int i = 0; i < request->Count; i++)
				{
					request[i].ToMT4(&reqsts[i]);
				}

				int* logs = new int[logins->Count];
				for (int i = 0; i < logins->Count; i++)
					logs[i] = logins[i];


				MetaQuotes::DailyReport* records = m_manager->DailyReportsRequest(reqsts, logs, &total);
				List<DailyReport>^ result = gcnew List<DailyReport>();
				for (int i = 0; i < total; i++) {
					result->Add(DailyReport(records[i]));
				}

				m_manager->MemFree(records);
				delete reqsts;
				delete[] logs;

				return result;
			}

			//--- external command
			RetValues ExternalCommand(String^ data_in, [Out] String^% data_out)
			{
				char* str_data_in = (char*)(void*)Marshal::StringToHGlobalAnsi(data_in);
				int in_len = data_in->Length;
				int out_len = 2048;
				char *str_data_out = NULL;

				int res = m_manager->ExternalCommand(str_data_in, in_len, &str_data_out, &out_len);

				if (res == MetaQuotes::RET_OK && out_len != 0)
					data_out = gcnew String(str_data_out, 0, out_len);
				else
					data_out = Marshal::PtrToStringAnsi(IntPtr("Execption"));

				if (str_data_out != NULL && out_len != 0)
					m_manager->MemFree(str_data_out);
				Marshal::FreeHGlobal(IntPtr((void*)str_data_in));
				return (RetValues)res;
			}

			//--- plugins
			RetValues PluginUpdate(ConPluginParam^ plugin)
			{
				MetaQuotes::ConPluginParam plg;
				plugin->ToMT4(&plg);
				return (RetValues)m_manager->PluginUpdate(&plg);
			}

			//--- pumping
			RetValues PumpingSwitch(PumpingDelegate ^pfnFunc)
			{

				if (gch.IsAllocated) {
					gch.Free();
				}
				gch = GCHandle::Alloc(pfnFunc);
				IntPtr ip = Marshal::GetFunctionPointerForDelegate(pfnFunc);
				MetaQuotes::MTAPI_NOTIFY_FUNC cb = static_cast<MetaQuotes::MTAPI_NOTIFY_FUNC>(ip.ToPointer());

				// force garbage collection cycle to prove
				// that the delegate doesn't get disposed
				GC::Collect();

				int res = m_manager->PumpingSwitch(cb, 0, NULL, 0);
				// release reference to delegate
				//gch.Free();
				return (RetValues)res;
			}


			List<ConGroup^>^ GroupsGet()//gets all groups
			{
				int total = 0;
				List<ConGroup^>^ list = gcnew List<ConGroup^>();
				MetaQuotes::ConGroup* groups = m_manager->GroupsGet(&total);
				for (int i = 0; i < total; i++)
				{
					list->Add(gcnew ConGroup(groups[i]));
				}
				m_manager->MemFree(groups);
				return list;
			}

			ConGroup^ GroupRecordGet(String^ group)
			{
				MetaQuotes::ConGroup grp;
				char *_group = (char *)(void *)Marshal::StringToHGlobalAnsi(group);
				m_manager->GroupRecordGet(_group, &grp);

				ConGroup^ gr = gcnew ConGroup(grp);
				Marshal::FreeHGlobal(IntPtr((void *)_group));
				return gr;
			}

			List<SymbolInfo>^ SymbolInfoUpdated()
			{
				List<SymbolInfo> ^res = gcnew List<SymbolInfo>();
				int total = 0;
				MetaQuotes::SymbolInfo si[32];
				while ((total = m_manager->SymbolInfoUpdated(si, 32)) > 0)
				{
					for (int i = 0; i < total; i++)
					{
						res->Add(SymbolInfo(si[i]));
					}
				}
				return res;
			}

			List<UserRecord>^ UsersGet()
			{
				MetaQuotes::UserRecord* ur;
				int total;
				ur = m_manager->UsersGet(&total);
				List<UserRecord>^ users = gcnew List<UserRecord>();
				for (int i = 0; i < total; i++)
				{
					UserRecord user = UserRecord(ur[i]);
					users->Add(user);
				}
				return users;
			}

			RetValues GetUser(int login, [Out]UserRecord %user)
			{
				int total = 0;
				MetaQuotes::UserRecord usr = { 0 };
				int res = m_manager->UserRecordGet(login, &usr);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				user = UserRecord(usr);

				return (RetValues)res;
			}

			List<OnlineRecord>^ OnlineGet()
			{
				//OnlineRecord* OnlineRequest(int *total)
				MetaQuotes::OnlineRecord* ur;
				int total;
				ur = m_manager->OnlineGet(&total);
				List<OnlineRecord>^ users = gcnew List<OnlineRecord>();
				for (int i = 0; i < total; i++)
				{
					OnlineRecord user = OnlineRecord(ur[i]);
					users->Add(user);
				}
				m_manager->MemFree(ur);
				return users;
			}


			RetValues OnlineRecordGet(int login, [Out]OnlineRecord  %user)
			{
				int total = 0;
				MetaQuotes::OnlineRecord usr = { 0 };
				int res = m_manager->OnlineRecordGet(login, &usr);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				user = OnlineRecord(usr);

				return (RetValues)res;
			}

			List<TradeRecord>^ TradesGet()
			{
				
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *trs = m_manager->TradesGet(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(trs[i]));
				}
				m_manager->MemFree(trs);
				return res;
			}

			List<TradeRecord>^ TradesGetBySymbol(String ^symbol)
			{
				char* _symbol = (char*)(void*)Marshal::StringToHGlobalAnsi(symbol);
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *trs = m_manager->TradesGetBySymbol(_symbol, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(trs[i]));
				}
				Marshal::FreeHGlobal(IntPtr((void*)_symbol));
				m_manager->MemFree(trs);
				return res;
			}

			List<TradeRecord>^ TradesGetByLogin(int login, String^ group)
			{
				char* _group = (char*)(void*)Marshal::StringToHGlobalAnsi(group);
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *trs = m_manager->TradesGetByLogin(login, _group, &total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(trs[i]));
				}
				Marshal::FreeHGlobal(IntPtr((void*)_group));
				m_manager->MemFree(trs);
				return res;
			}

			List<TradeRecord>^ TradesGetByMarket()
			{

				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *trs = m_manager->TradesGetByMarket(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(trs[i]));
				}
				m_manager->MemFree(trs);
				return res;
			}


			RetValues TradeRecordGet(int order, [Out]TradeRecord %tr)
			{
				int total = 0;
				MetaQuotes::TradeRecord trs = { 0 };
				int res = m_manager->TradeRecordGet(order, &trs);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				tr = TradeRecord(trs);

				return (RetValues)res;
			}

			RetValues TradeClearRollback(int order)
			{
				return (RetValues)m_manager->TradeClearRollback(order);
			}


			List<MarginLevel>^ MarginsGet()
			{
				List<MarginLevel>^ AllMargins = gcnew List<MarginLevel>();
				int total = 0;
				MetaQuotes::MarginLevel* margins = m_manager->MarginsGet(&total);
				for (int i = 0; i < total; i++)
				{
					MarginLevel marg = MarginLevel(margins[i]);
					AllMargins->Add(marg);

				}
				m_manager->MemFree(margins);
				return AllMargins;
			}

			MarginLevel MarginLevelGet(int login, String^ group)
			{
				MetaQuotes::MarginLevel margin;
				char *_group = (char *)(void *)Marshal::StringToHGlobalAnsi(group);
				m_manager->MarginLevelGet(login, _group, &margin);

				MarginLevel marginlevel = MarginLevel(margin);

				Marshal::FreeHGlobal(IntPtr((void *)_group));
				return marginlevel;
			}

			List<RequestInfo>^ RequestsGet()
			{
				List<RequestInfo>^ requests = gcnew List<RequestInfo>();
				int total = 0;
				MetaQuotes::RequestInfo* req = m_manager->RequestsGet(&total);
				for (int i = 0; i < total; i++)
				{
					RequestInfo requ = RequestInfo(req[i]);
					requests->Add(requ);

				}
				m_manager->MemFree(req);
				return requests;
			}

			RetValues RequestInfoGet(int pos, [Out]RequestInfo %info)
			{
				int total = 0;
				MetaQuotes::RequestInfo inf = { 0 };
				int res = m_manager->RequestInfoGet(pos, &inf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				info = RequestInfo(inf);

				return (RetValues)res;
			}

			List<ConPlugin>^ PluginsGet()
			{
				List<ConPlugin>^ plugins = gcnew List<ConPlugin>();
				int total = 0;
				MetaQuotes::ConPlugin* plg = m_manager->PluginsGet(&total);
				for (int i = 0; i < total; i++)
				{
					ConPlugin plgs = ConPlugin(plg[i]);
					plugins->Add(plgs);

				}

				m_manager->MemFree(plg);
				return plugins;
			}

			RetValues PluginParamGet(int pos, [Out]ConPluginParam %plugin)
			{
				MetaQuotes::ConPluginParam con = { 0 };
				int res = m_manager->PluginParamGet(pos, &con);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				plugin = ConPluginParam(con);

				return (RetValues)res;
			}

			RetValues MailLast(String^ group, [Out]int %length)
			{
				int total = 0;
				char *_group = (char *)(void *)Marshal::StringToHGlobalAnsi(group);
				int res = m_manager->MailLast(_group, &total);
				Marshal::FreeHGlobal(IntPtr((void *)_group));
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				length = total;

				return (RetValues)res;
			}

			List<NewsTopic>^ NewsGet()
			{
				MetaQuotes::NewsTopic*   news;
				int total;
				news = m_manager->NewsGet(&total);

				List<NewsTopic>^ allnews = gcnew List<NewsTopic>();
				for (int i = 0; i < total; i++)
				{
					m_manager->NewsBodyRequest(news[i].key);
					NewsTopic ntopic = NewsTopic(news[i]);
					allnews->Add(ntopic);
				}
				m_manager->MemFree(news);
				return allnews;
			}

			int NewsTotal() {
				return m_manager->NewsTotal();
			}

			RetValues NewsTopicGet(int pos, [Out]NewsTopic %news)
			{

				MetaQuotes::NewsTopic mt4news = { 0 };
				int res = m_manager->NewsTopicGet(pos, &mt4news);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				news = NewsTopic(mt4news);

				return (RetValues)res;
			}

			void NewsBodyRequest(int key) {
				m_manager->NewsBodyRequest(key);
			}

			String^ NewsBodyGet(int key) {

				char *news = m_manager->NewsBodyGet(key);
				return Marshal::PtrToStringAnsi(IntPtr(news));
			}

			//--- dealing

			RetValues DealerSwitch(PumpingDelegate ^pfnFunc)
			{

				if (gch.IsAllocated) {
					gch.Free();
				}
				gch = GCHandle::Alloc(pfnFunc);
				IntPtr ip = Marshal::GetFunctionPointerForDelegate(pfnFunc);
				MetaQuotes::MTAPI_NOTIFY_FUNC cb = static_cast<MetaQuotes::MTAPI_NOTIFY_FUNC>(ip.ToPointer());

				// force garbage collection cycle to prove
				// that the delegate doesn't get disposed
				GC::Collect();

				int res = m_manager->DealerSwitch(cb, 0, NULL);
				// release reference to delegate
				//gch.Free();
				return (RetValues)res;
			}


			RetValues  DealerRequestGet([Out]RequestInfo %info) {

				MetaQuotes::RequestInfo inf = { 0 };
				int res = m_manager->DealerRequestGet(&inf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				info = RequestInfo(inf);

				return (RetValues)res;

			}



			RetValues DealerSend([Out]RequestInfo %info, int requote, int mode) {
			
				MetaQuotes::RequestInfo inf = { 0 };
				info.ToMT4(&inf);
				int res = m_manager->DealerSend(&inf,requote,mode);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;

				info = RequestInfo(inf);

				return (RetValues)res;
			
			}

			RetValues DealerReject(int id) {
				return (RetValues)m_manager->DealerReject(id);
			}

			RetValues DealerReset(int id) {
				return (RetValues)m_manager->DealerReset(id);
			}

			//---
			List<TickInfo> ^TickInfoLast(String^ symbol)
			{
				List<TickInfo> ^result = gcnew List<TickInfo>();
				int total = 0;
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);
				MetaQuotes::TickInfo *tr = m_manager->TickInfoLast(_symbol, &total);
				for (int i = 0; i < total; i++) {
					result->Add(TickInfo(tr[i]));
				}
				m_manager->MemFree(tr);
				Marshal::FreeHGlobal(IntPtr((void *)_symbol));
				return result;
			}

			List<ConSymbolGroup>^ SymbolsGroupsGet()
			{
				int total = 0;
				List<ConSymbolGroup> ^res = gcnew List<ConSymbolGroup>();
				MetaQuotes::ConSymbolGroup SymbolGroupsArray[MAX_SEC_GROUP];

				m_manager->SymbolsGroupsGet(SymbolGroupsArray);
					

				for (int i = 0; i < MAX_SEC_GROUP; i++)
				{
					MetaQuotes::ConSymbolGroup &csg = SymbolGroupsArray[i];
					ConSymbolGroup c = ConSymbolGroup(csg);
					c.index = i;
					res->Add(c);
				}

				return res;
			}

			UInt32 GetServerTime()
			{
				return m_manager->ServerTime();
				
			}

			//virtual MailBox*     __stdcall MailsRequest(int *total) = 0;
			List<MailBox> ^MailsRequest()
			{
				List<MailBox> ^result = gcnew List<MailBox>();
				int total = 0;
				MetaQuotes::MailBox *mail = m_manager->MailsRequest(&total);
				for (int i = 0; i < total; i++) {
					result->Add(MailBox(mail[i]));
				}
				m_manager->MemFree(mail);
				return result;
			}

			//--- risk management
			List<SymbolSummary> ^SummaryGetAll()
			{
				List<SymbolSummary> ^result = gcnew List<SymbolSummary>();
				int total = 0;
				MetaQuotes::SymbolSummary *symbols = m_manager->SummaryGetAll(&total);
				for (int i = 0; i < total; i++) {
					result->Add(SymbolSummary(symbols[i]));
				}
				m_manager->MemFree(symbols);
				return result;
			}

			SymbolSummary SummaryGet(String^ symbol)
			{
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);
				MetaQuotes::SymbolSummary ss = { 0 };
				m_manager->SummaryGet(_symbol, &ss);
				SymbolSummary res = SymbolSummary(ss);
				Marshal::FreeHGlobal(IntPtr((void *)_symbol));
				return res;
			}

			RetValues SummaryGetByCount(int symbol, [Out] SymbolSummary %info) {

				MetaQuotes::SymbolSummary inf = { 0 };
				int res = m_manager->SummaryGetByCount(symbol,&inf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				info = SymbolSummary(inf);
				return (RetValues)res;
			}

			RetValues SummaryGetByType(int symbol, [Out] SymbolSummary %info) {

				MetaQuotes::SymbolSummary inf = { 0 };
				int res = m_manager->SummaryGetByType(symbol, &inf);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				info = SymbolSummary(inf);
				return (RetValues)res;
			}

			RetValues SummaryCurrency(String^ cur, int maxchars) {
				char * _cur = (char *)(void *)Marshal::StringToHGlobalAnsi(cur);
				int res=m_manager->SummaryCurrency(_cur, maxchars);
				Marshal::FreeHGlobal(IntPtr((void *)_cur));
				return (RetValues)res;

			}

			List<ExposureValue> ^ExposureGet()
			{
				List<ExposureValue> ^result = gcnew List<ExposureValue>();
				int total = 0;
				MetaQuotes::ExposureValue *exp = m_manager->ExposureGet(&total);
				for (int i = 0; i < total; i++) {
					result->Add(ExposureValue(exp[i]));
				}
				m_manager->MemFree(exp);
				return result;
			}

			RetValues ExposureValueGet(String^ cur, [Out] ExposureValue %info) {

				MetaQuotes::ExposureValue inf = { 0 };
				char * _cur = (char *)(void *)Marshal::StringToHGlobalAnsi(cur);
				int res = m_manager->ExposureValueGet(_cur, &inf);
				Marshal::FreeHGlobal(IntPtr((void *)_cur));
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				info = ExposureValue(inf);
				return (RetValues)res;
			}

			//---
			RetValues MarginLevelRequest(int login, [Out] MarginLevel %level) {

				MetaQuotes::MarginLevel lvl = { 0 };
				int res = m_manager->MarginLevelRequest(login, &lvl);
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				level = MarginLevel(lvl);
				return (RetValues)res;
			}

			RetValues HistoryCorrect(String^ symbol, [Out] int %updated) {
			
				int updt;
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);
				int res=m_manager->HistoryCorrect(_symbol, &updt);
				Marshal::FreeHGlobal(IntPtr((void *)_symbol));
				if (res != MetaQuotes::RET_OK)
					return (RetValues)res;
				updated = updt;
				return (RetValues)res;

			}

			//--- new chart bases
			List<RateInfo> ^ChartRequest(List<ChartInfo> ^ chart, [Out] int %timesign)
			{
				__time32_t stamp = 0;
				MetaQuotes::ChartInfo* m_chart = new MetaQuotes::ChartInfo[chart->Count];
				for (int i = 0; i < chart->Count; i++)
				{
					chart[i].ToMT4(&m_chart[i]);
				}
				List<RateInfo> ^res = gcnew List<RateInfo>();
				int total = 0;
				MetaQuotes::RateInfo *ri = m_manager->ChartRequest(m_chart, &stamp, &total);
				for (int i = 0; i < total; i++) {
					res->Add(RateInfo(ri[i]));
				}
				timesign = stamp;
				m_manager->MemFree(ri);
				delete[] m_chart;
				return res;
			}

			RetValues ChartAdd(String ^symbol, ChartPeriod cp, List<RateInfo> ^ris)
			{
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);

				MetaQuotes::RateInfo *m = new MetaQuotes::RateInfo[ris->Count];
				int count = 0;

				for each(RateInfo ri in ris) {
					 ri.ToMT4(&m[count++]);
				}

				int res = m_manager->ChartAdd(_symbol, (const int)cp, m, &count);
				delete[] m;

				Marshal::FreeHGlobal(IntPtr((void *)_symbol));

				return (RetValues)res;
			}


			RetValues ChartUpdate(String ^symbol, ChartPeriod cp, List<RateInfo> ^ris)
			{
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);

				MetaQuotes::RateInfo *m = new MetaQuotes::RateInfo[ris->Count];
				int count = 0;
				for each(RateInfo ri in ris) {
					ri.ToMT4(&m[count++]);
				}
				int res = m_manager->ChartUpdate(_symbol, (const int)cp, m, &count);
				delete[] m;

				Marshal::FreeHGlobal(IntPtr((void *)_symbol));

				return (RetValues)res;
			}

			RetValues ChartDelete(String ^symbol, ChartPeriod cp, List<RateInfo> ^ris)
			{
				char *_symbol = (char *)(void *)Marshal::StringToHGlobalAnsi(symbol);

				MetaQuotes::RateInfo *m = new MetaQuotes::RateInfo[ris->Count];
				int count = 0;

				for each(RateInfo ri in ris) {
					ri.ToMT4(&m[count++]);
				}
				int res = m_manager->ChartDelete(_symbol, (const int)cp, m, &count);
				delete[] m;

				Marshal::FreeHGlobal(IntPtr((void *)_symbol));

				return (RetValues)res;
			}

			//--- ticks base
			List<TickRecord> ^TicksRequest(TickRequest ^ request)
			{
			
				MetaQuotes::TickRequest* m_ticks = new MetaQuotes::TickRequest;
			



				List<TickRecord> ^res = gcnew List<TickRecord>();
				int total = 0;

				MetaQuotes::TickRecord *ri = m_manager->TicksRequest(m_ticks, &total);
				

				for (int i = 0; i < total; i++) {
					res->Add(TickRecord(ri[i]));
				}

				m_manager->MemFree(ri);
				delete[] m_ticks;

				return res;
			}

			//--- internal methods
			RetValues PumpingSwitchEx(PumpingExDelegate ^pfnFunc,  int flags, int param)
			{

				if (gch.IsAllocated) {
					gch.Free();
				}
				gch = GCHandle::Alloc(pfnFunc);
				IntPtr ip = Marshal::GetFunctionPointerForDelegate(pfnFunc);
				MetaQuotes::MTAPI_NOTIFY_FUNC_EX cb = static_cast<MetaQuotes::MTAPI_NOTIFY_FUNC_EX>(ip.ToPointer());

				// force garbage collection cycle to prove
				// that the delegate doesn't get disposed
				GC::Collect();

				int res = m_manager->PumpingSwitchEx(cb, flags, (void *)param);
				// release reference to delegate
				//gch.Free();
				return (RetValues)res;
			}

			RetValues UsersSyncStart(UInt32 timestamp)
			{
				return (RetValues)m_manager->UsersSyncStart(timestamp);
			}

			List<UserRecord>^ UsersSyncRead()
			{
				List<UserRecord> ^res = gcnew List<UserRecord>();
				int total = 0;
				MetaQuotes::UserRecord *ur = NULL;
				ur = m_manager->UsersSyncRead(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(UserRecord(ur[i]));
				}
				m_manager->MemFree(ur);
				return res;
			}

			List<int>^ UsersSnapshot()
			{
				List<int>^ res = gcnew List<int>();
				int total = 0;
				int *logins = NULL;
				logins = m_manager->UsersSnapshot(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(logins[i]);
				}
				delete logins;
				return res;
			}

			RetValues TradesSyncStart(UInt32 timestamp)
			{
				return (RetValues)m_manager->TradesSyncStart(timestamp);
			}

			List<TradeRecord>^ TradesSyncRead()
			{
				List<TradeRecord> ^res = gcnew List<TradeRecord>();
				int total = 0;
				MetaQuotes::TradeRecord *tr = NULL;
				tr = m_manager->TradesSyncRead(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(TradeRecord(tr[i]));
				}
				m_manager->MemFree(tr);
				return res;
			}

			List<int>^ TradesSnapshot()
			{
				List<int>^ res = gcnew List<int>();
				int total = 0;
				int *orders = NULL;
				orders = m_manager->TradesSnapshot(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(orders[i]);
				}
				m_manager->MemFree(orders);
				return res;
			}


			RetValues DailySyncStart(UInt32 timestamp)
			{
				return (RetValues)m_manager->DailySyncStart(timestamp);
			}

			List<DailyReport>^ DailySyncRead()
			{
				List<DailyReport> ^res = gcnew List<DailyReport>();
				int total = 0;
				MetaQuotes::DailyReport *dr = NULL;
				dr = m_manager->DailySyncRead(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(DailyReport(dr[i]));
				}
				m_manager->MemFree(dr);
				return res;
			}

			//--- profit recalculation
			RetValues TradeCalcProfit([Out]TradeRecord %tr)
			{
				MetaQuotes::TradeRecord trade = { 0 };

				tr.ToMT4(&trade);

				int res = m_manager->TradeCalcProfit(&trade);
				if (res == MetaQuotes::RET_OK)
					tr.profit = trade.profit;
				return (RetValues)res;
			}

			//--- new symbol commands
			RetValues SymbolChange(SymbolProperties ^prop)
			{
				MetaQuotes::SymbolProperties symbol = { 0 };
				prop->ToMT4(&symbol);
				return (RetValues)m_manager->SymbolChange(&symbol);		
			}

			//--- network statistics
			RetValues BytesSent()
			{
				return (RetValues)m_manager->BytesSent();
			}

			RetValues BytesReceived()
			{
				return (RetValues)m_manager->BytesReceived();
			}

			//---
			RetValues ManagerCommon([Out] ConCommon %conCom)
			{

				MetaQuotes::ConCommon conC;
				
				int res = m_manager->ManagerCommon(&conC);

				if (res == MetaQuotes::RET_OK)
					conCom = ConCommon(conC);

				return (RetValues)res;
			}

			//--- log access

			void LogsOut(int code, String^ source, String^ msg)
			{
				char* _source=(char *)(void *)Marshal::StringToHGlobalAnsi(source);
				char* _msg = (char *)(void *)Marshal::StringToHGlobalAnsi(msg);
				m_manager->LogsOut(code, _source, _msg);
				Marshal::FreeHGlobal(IntPtr((void *)_source));
				Marshal::FreeHGlobal(IntPtr((void *)_msg));
			}

			void LogsMode(int mode)
			{
				m_manager->LogsMode(mode);
			}

			//--- check license
			RetValues LicenseCheck(String^ license_name)
			{
				char* licensename=(char *)(void *)Marshal::StringToHGlobalAnsi(license_name);
				int res=m_manager->LicenseCheck(licensename);
				Marshal::FreeHGlobal(IntPtr((void *)licensename));
				return (RetValues)res;
			}

			//--- gateway configs
			List<ConGatewayAccount>^ CfgRequestGatewayAccount()
			{
				List<ConGatewayAccount> ^res = gcnew List<ConGatewayAccount>();
				int total = 0;
				MetaQuotes::ConGatewayAccount *con = NULL;
				con = m_manager->CfgRequestGatewayAccount(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConGatewayAccount(con[i]));
				}
				m_manager->MemFree(con);
				return res;
			}

			List<ConGatewayMarkup>^ CfgRequestGatewayMarkup()
			{
				List<ConGatewayMarkup> ^res = gcnew List<ConGatewayMarkup>();
				int total = 0;
				MetaQuotes::ConGatewayMarkup *con = NULL;
				con = m_manager->CfgRequestGatewayMarkup(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConGatewayMarkup(con[i]));
				}
				m_manager->MemFree(con);
				return res;
			}

			List<ConGatewayRule>^ CfgRequestGatewayRule()
			{
				List<ConGatewayRule> ^res = gcnew List<ConGatewayRule>();
				int total = 0;
				MetaQuotes::ConGatewayRule *con = NULL;
				con = m_manager->CfgRequestGatewayRule(&total);
				for (int i = 0; i < total; i++)
				{
					res->Add(ConGatewayRule(con[i]));
				}
				m_manager->MemFree(con);
				return res;
			}
			

			//--- configuration update
			RetValues CfgUpdateGatewayAccount(ConGatewayAccount ^cfg)
			{
				MetaQuotes::ConGatewayAccount con = { 0 };
				cfg->ToMT4(&con);
				return (RetValues)m_manager->CfgUpdateGatewayAccount(&con);
			}

			RetValues CfgUpdateGatewayMarkup(ConGatewayMarkup ^cfg)
			{
				MetaQuotes::ConGatewayMarkup con = { 0 };
				cfg->ToMT4(&con);
				return (RetValues)m_manager->CfgUpdateGatewayMarkup(&con);
			}

			RetValues CfgUpdateGatewayRule(ConGatewayRule ^cfg)
			{
				MetaQuotes::ConGatewayRule con = { 0 };
				cfg->ToMT4(&con);
				return (RetValues)m_manager->CfgUpdateGatewayRule(&con);
			}

			//--- configuration delete
			RetValues CfgDeleteGatewayAccount(int pos)
			{
				return (RetValues)m_manager->CfgDeleteGatewayAccount(pos);
			}
			RetValues CfgDeleteGatewayMarkup(int pos)
			{
				return (RetValues)m_manager->CfgDeleteGatewayMarkup(pos);
			}
			RetValues CfgDeleteGatewayRule(int pos)
			{
				return (RetValues)m_manager->CfgDeleteGatewayRule(pos);
			}

			//--- configuration shift
			RetValues CfgShiftGatewayAccount(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftGatewayAccount(pos, shift);
			}

			RetValues CfgShiftGatewayMarkup(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftGatewayMarkup(pos, shift);
			}

			RetValues CfgShiftGatewayRule(int pos, int shift)
			{
				return (RetValues)m_manager->CfgShiftGatewayRule(pos, shift);
			}
	
			//--- administrator databases commands
			//virtual BalanceDiff* __stdcall AdmBalanceCheck(int *logins, int *total) = 0;
			List<BalanceDiff>^ AdmBalanceCheck(List<int>^ logins)
			{
				int total = logins->Count;
				int* usr = new int[total];
				List<BalanceDiff> ^res = gcnew List<BalanceDiff>();

				for (int i = 0; i < logins->Count; i++)
				{
					usr[i] = logins[i];
				}

				MetaQuotes::BalanceDiff* bal = m_manager->AdmBalanceCheck(usr, &total);

				for (int i = 0; i < total; i++)
				{
					res->Add(BalanceDiff(bal[i]));
				}
				delete[]usr;
				m_manager->MemFree(bal);
				return res;
			}

			//--- notifications
			RetValues NotificationsSend( String^ metaquotes_ids, String^ message)
			{
				wchar_t *metaquotesids=(wchar_t *)(void *)Marshal::StringToHGlobalAnsi(metaquotes_ids);
				wchar_t *_message = (wchar_t *)(void *)Marshal::StringToHGlobalAnsi(message);
				int res=m_manager->NotificationsSend(metaquotesids, _message);
				Marshal::FreeHGlobal(IntPtr((void *)metaquotesids));
				Marshal::FreeHGlobal(IntPtr((void *)_message));
				return (RetValues)res;

			}

			RetValues NotificationsSend(List<int>^ logins, unsigned int logins_total, String^ message)
			{
				int total = logins->Count;
				int* usr = new int[total];

				for (int i = 0; i < logins->Count; i++)
				{
					usr[i] = logins[i];
				}
				wchar_t *_message = (wchar_t *)(void *)Marshal::StringToHGlobalAnsi(message);
				int res= m_manager->NotificationsSend(usr, logins_total, _message);
				delete[] usr;
				Marshal::FreeHGlobal(IntPtr((void *)_message));
				return (RetValues)res;
			}

			TradeRecord DataToTradeRecord(IntPtr data)
			{
				if (data != IntPtr::Zero)
					return (TradeRecord(*(MetaQuotes::TradeRecord*)(void*)data));
			}
			UserRecord DataToUserRecord(IntPtr data)
			{
				if (data != IntPtr::Zero)
					return (UserRecord(*(MetaQuotes::UserRecord*)(void*)data));
			}


		};


	
}