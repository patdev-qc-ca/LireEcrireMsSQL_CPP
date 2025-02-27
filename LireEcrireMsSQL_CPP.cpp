// LireEcrireMsSQL_CPP.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream> //flux std i/o
#include <windows.h>
#include <sqlext.h>
#include<cstdio>
/*
#if defined(_WIN64)
    #if defined(_MSC_VER)
#error Ce programme ne peut etre compilé en 64 car il comporte des segents 16bits
    #endif
#endif
*/
#pragma warning(disable : 4996)
#pragma warning(disable:2011)
#pragma warning(disable:4005)
/// <summary>
/// SQL
/// </summary>
SQLHANDLE sqlEnvHandle;
SQLHANDLE sqlConnHandle;
SQLHANDLE sqlStmtHandle;
/// <summary>
/// Access
/// </summary>
SQLHANDLE mdbEnvHandle; 
SQLHANDLE mdbConnHandle; 
SQLHANDLE mdbStmtHandle;
/// <summary>
/// Commun
/// </summary>
SQLCHAR message[1024];
SQLRETURN retCode;
SQLCHAR SQLState[1024];

CHAR computerName[MAX_COMPUTERNAME_LENGTH + 1]; 
/// <summary>
/// il existe bien d'autres champs dans cette table systeme
/// </summary>
class ScanneTableSQL {
public:
    SQLCHAR TABLE_CATALOG[0x19];
    SQLCHAR TABLE_SCHEMA[0x03];
    SQLCHAR TABLE_NAME[0x32];
    SQLCHAR COLUMN_NAME[0x30];
    SQLINTEGER ORDINAL_POSITION;
    SQLINTEGER IS_NULLABLE;
    SQLCHAR DATA_TYPE[0x0F];
    SQLINTEGER CHARACTER_MAXIMUM_LENGTH;
};
class TablesSQL {
public:
    class Employes {
    public:
        SQLINTEGER noemploye;
        SQLCHAR loginname[0x32];
        SQLCHAR passwd[0x32];
        SQLCHAR employe[0x32];
        SQLCHAR initiale[0x32];
        SQLCHAR tel[0x32];
        SQLCHAR page[0x32];
        SQLCHAR cell[0x32];
        SQLINTEGER Actif;
        SQLINTEGER groupe;
        SQLINTEGER Famille;
        SQLINTEGER Supprime;
    };
    Employes emp;
};
TablesSQL tsql;
int ObtenirNomPC()
{
    DWORD size = sizeof(computerName) / sizeof(computerName[0]); 
    if (GetComputerNameA(computerName, &size)) { std::cout << "Computer Name: " << computerName << std::endl; }
    else { std::cerr << "Error: " << GetLastError() << std::endl; }    
    return 0;
}
void AfficherErreur(unsigned int handletype, const SQLHANDLE& handle) {
    if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, SQLState, NULL, message, 1024, NULL))
        std::cout << "Information: " << message << "\nÉtatSQL: " << SQLState << "." << std::endl;
}
int ConnexionMDB(char * Fichier) 
{
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))return -1;
    if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0))return -1;
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))return -1;
    SQLCHAR retConString[1024];
    char mdbstring[MAX_PATH];
    sprintf(mdbstring, "DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%s;", Fichier);
    switch (SQLDriverConnect(sqlConnHandle, NULL, (SQLCHAR*)mdbstring, SQL_NTS, retConString, 1024, NULL, SQL_DRIVER_NOPROMPT)) 
    {
        case SQL_SUCCESS_WITH_INFO:AfficherErreur(SQL_HANDLE_DBC, sqlConnHandle);break;
        case SQL_INVALID_HANDLE:
        case SQL_ERROR:AfficherErreur(SQL_HANDLE_DBC, sqlConnHandle);return -1;
        default:break;
    }
    std::cout << "Base Access: " <<Fichier<< std::endl;
}
SQLRETURN DeconnexionMDB()
{
    SQLFreeHandle(SQL_HANDLE_STMT, mdbStmtHandle);
    SQLDisconnect(mdbConnHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, mdbConnHandle);
    return SQLFreeHandle(SQL_HANDLE_ENV, mdbEnvHandle);
}
int ConnexionSQL(const char* NomServeur, const char* NomDatabase, const char* NomUtilisateur, const char* MotPasse)
{
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))              return -1;
    if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0))  return -1;
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))                return -1;
    SQLCHAR retConString[1024];
    CHAR constr[MAX_PATH];
    wsprintf(constr, "DRIVER={SQL Server};SERVER=%s\\SQLEXPRESS;DATABASE=%s;UID=%s;PWD=%s;",NomServeur,NomDatabase,NomUtilisateur,MotPasse);
    switch (SQLDriverConnect(sqlConnHandle, NULL, (SQLCHAR*)constr, SQL_NTS, retConString, 1024, NULL, SQL_DRIVER_NOPROMPT))
    {
        case SQL_SUCCESS_WITH_INFO:     AfficherErreur(SQL_HANDLE_DBC, sqlConnHandle);        break;
        case SQL_INVALID_HANDLE:
        case SQL_ERROR:                 AfficherErreur(SQL_HANDLE_DBC, sqlConnHandle);        return -1;
        default:        break;
    }
    std::cout <<NomUtilisateur << " est connecte au serveur: " <<NomServeur<< std::endl;
}
SQLRETURN DeconnexionSQL() 
{
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    SQLDisconnect(sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
    return SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
}
SQLRETURN LireDonneesSQL(SQLHSTMT Handle, SQLUSMALLINT Ordinal, SQLSMALLINT Type, _Out_writes_opt_(_Inexpressible_(TailleTampon))
    SQLPOINTER TargetValue, SQLLEN TailleTampon, _Out_opt_ SQLINTEGER* Longueur)
{
#if defined(_WIN64)
    return SQLGetData(Handle, Ordinal, Type, TargetValue, TailleTampon, (SQLLEN*)Longueur);
#else
    return SQLGetData(Handle, Ordinal, Type, TargetValue, TailleTampon, Longueur);
#endif

}
int LireDonneesSQL(const char* Requete) {
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))        return -1;
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)Requete, SQL_NTS)) 
    {
        AfficherErreur(SQL_HANDLE_STMT, sqlStmtHandle);
        return -1;
    }else {
        SQLCHAR sqlVersion[MAX_PATH];//définit a 0x100 par windef
        SQLINTEGER ptrSqlVersion;
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) 
        {
            #if defined(_WIN64)
                SQLGetData(sqlStmtHandle, 1, SQL_C_DEFAULT, &sqlVersion, sizeof(sqlVersion), (SQLLEN *)&ptrSqlVersion);
            #else
                SQLGetData(sqlStmtHandle, 1, SQL_C_DEFAULT, &sqlVersion, sizeof(sqlVersion), &ptrSqlVersion);
            #endif
            std::cout << "SQL VERSION: " << sqlVersion << std::endl;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    return 0;
}
SQLRETURN LireDonneesSQL(SQLHSTMT Handle, SQLUSMALLINT Ordinal, SQLSMALLINT Type, _Out_writes_opt_(_Inexpressible_(TailleTampon))
    SQLPOINTER TargetValue)
{
    return SQLGetData(Handle, Ordinal, Type, TargetValue, 50, NULL);
}
void ListerTableDuCatalogue() 
{
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))        return;
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)"SELECT NAME AS 'NOM', OBJECT_ID AS 'IDENTIFIANT', TYPE_DESC AS 'TYPE', CREATE_DATE AS 'CREATION', MODIFY_DATE AS 'MODIFICATION', MAX_COLUMN_ID_USED AS 'COLONES' FROM SYS.TABLES WHERE TYPE = 'U' ORDER BY NAME", SQL_NTS))
    {
        AfficherErreur(SQL_HANDLE_STMT, sqlStmtHandle);
        return;
    }
    else {
        SQLCHAR Nom[40];
        SQLDOUBLE Identifiant;
        SQLCHAR Type;
        SQLINTEGER Colones;
        FILE* f = fopen("ListeDesTables.txt", "wb");
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS)
        {
            LireDonneesSQL(sqlStmtHandle, 1, SQL_C_DEFAULT, &Nom, sizeof(Nom), NULL);
            LireDonneesSQL(sqlStmtHandle, 2, SQL_C_DEFAULT, &Identifiant, sizeof(Identifiant), NULL);
            LireDonneesSQL(sqlStmtHandle, 3, SQL_C_DEFAULT, &Type, sizeof(Type), NULL);
            LireDonneesSQL(sqlStmtHandle, 6, SQL_C_DEFAULT, &Colones, sizeof(Colones), NULL);
            printf("Identifiant:%ld\tType:U\tColones:%d\t%s\n", Identifiant, Colones, Nom);
            fprintf(f,"Identifiant:%ld\tType:U\tColones:%d\t%s\n", Identifiant, Colones, Nom);
        }
        fclose(f);
    }
}
void ListerEmployes() {
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))        return;
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)"SELECT * FROM EMPLOYES ORDER BY ID", SQL_NTS))
    {AfficherErreur(SQL_HANDLE_STMT, sqlStmtHandle);return;}
    else {
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS)
        {
            LireDonneesSQL(sqlStmtHandle, 1, SQL_C_DEFAULT, &tsql.emp.noemploye);
            LireDonneesSQL(sqlStmtHandle, 2, SQL_C_DEFAULT, &tsql.emp.loginname);
            LireDonneesSQL(sqlStmtHandle, 3, SQL_C_DEFAULT, &tsql.emp.passwd);
            LireDonneesSQL(sqlStmtHandle, 4, SQL_C_DEFAULT, &tsql.emp.employe);
            LireDonneesSQL(sqlStmtHandle, 5, SQL_C_DEFAULT, &tsql.emp.initiale);
            LireDonneesSQL(sqlStmtHandle, 6, SQL_C_DEFAULT, &tsql.emp.tel);
            LireDonneesSQL(sqlStmtHandle, 7, SQL_C_DEFAULT, &tsql.emp.page);
            LireDonneesSQL(sqlStmtHandle, 8, SQL_C_DEFAULT, &tsql.emp.cell);
            LireDonneesSQL(sqlStmtHandle, 9, SQL_C_DEFAULT, &tsql.emp.Actif);
            LireDonneesSQL(sqlStmtHandle, 10, SQL_C_DEFAULT, &tsql.emp.groupe);
            LireDonneesSQL(sqlStmtHandle, 11, SQL_C_DEFAULT, &tsql.emp.Supprime);
            printf("%d\t%s\t%s\t%s\t%s\t%s\t%s\t%c\t%d\t%lc\t%ld\n", tsql.emp.noemploye,tsql.emp.employe,tsql.emp.loginname, tsql.emp.employe, tsql.emp.initiale,
                tsql.emp.tel, tsql.emp.page, tsql.emp.cell, tsql.emp.Actif, tsql.emp.groupe, tsql.emp.Actif);
        }
    }

}
void AnalyseTableSQL()
{
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle)) return;
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)"SELECT TABLE_CATALOG, TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME, \
        ORDINAL_POSITION, IS_NULLABLE, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH FROM INFORMATION_SCHEMA.COLUMNS \
        order by TABLE_NAME", SQL_NTS))
    {
        AfficherErreur(SQL_HANDLE_STMT, sqlStmtHandle); return;
    }
    else {
        ScanneTableSQL scnTbl;
        FILE* f= fopen("Structure_Allocation_Table.txt", "wb");
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            LireDonneesSQL(sqlStmtHandle, 1, SQL_C_DEFAULT, &scnTbl.TABLE_CATALOG);
            LireDonneesSQL(sqlStmtHandle, 2, SQL_C_DEFAULT, &scnTbl.TABLE_SCHEMA);
            LireDonneesSQL(sqlStmtHandle, 3, SQL_C_DEFAULT, &scnTbl.TABLE_NAME);
            LireDonneesSQL(sqlStmtHandle, 4, SQL_C_DEFAULT, &scnTbl.COLUMN_NAME);
            LireDonneesSQL(sqlStmtHandle, 5, SQL_C_DEFAULT, &scnTbl.ORDINAL_POSITION);
            fprintf(f,"Pos C++:%d\tC#: %d\t[dbo].[%s].[%s].[%s]\n",  scnTbl.ORDINAL_POSITION, scnTbl.ORDINAL_POSITION-1, scnTbl.TABLE_CATALOG, scnTbl.TABLE_NAME,
                scnTbl.COLUMN_NAME);
            printf( "Pos C++:%d\tC#: %d\t[dbo].[%s].[%s].[%s]\n", scnTbl.ORDINAL_POSITION, scnTbl.ORDINAL_POSITION - 1, scnTbl.TABLE_CATALOG, scnTbl.TABLE_NAME,
                scnTbl.COLUMN_NAME);
        }
        fclose(f);
    }
}

int main() {
    ObtenirNomPC();
    ConnexionSQL(computerName, "Master", "sa", "Password01$");
    ListerTableDuCatalogue();
    AnalyseTableSQL();
    DeconnexionSQL();
}

