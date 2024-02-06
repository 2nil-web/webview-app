
#include <iostream>
#include <windows.h>
#include <winldap.h>
#include <winber.h>

// This subroutine must have validated credentials (name and password) passed to it.
int MyLDAPSearch(std::string ip, int port, std::string binddn, std::string password, std::string base, std::string filter)
{
  // Initialize a session. LDAP_PORT is the default port, 389.
  LDAP* pLdapConnection=NULL;
  std::cout << "ip:" << ip << ", port:" << port << std::endl;
  pLdapConnection=ldap_init((PCHAR)(ip.c_str()), port);
  
  if (pLdapConnection == NULL) {
   std::cerr << "ldap_init failed with error " << std::hex << LdapGetLastError() << std::endl;
   ldap_unbind(pLdapConnection);
   return -1;
  }
  
  // Set session options.
  ULONG version=LDAP_VERSION3;
  ULONG numReturns=10;
  ULONG lRtn=0;
  
  // Set the version to 3.0 (default is 2.0).
/*
  lRtn=ldap_set_option( pLdapConnection, LDAP_OPT_PROTOCOL_VERSION, (void*) &version);

  if (lRtn != LDAP_SUCCESS) {
    std::cout << "SetOption Error: " << std::hex << lRtn << std::endl;
    ldap_unbind(pLdapConnection);
    return -1;
  }
*/
  // Set the limit on the number of entries returned to 10.

  lRtn=ldap_set_option( pLdapConnection, LDAP_OPT_SIZELIMIT, (void*) &numReturns);

  if (lRtn != LDAP_SUCCESS) {
    std::cout << "SetOption Error: " << std::hex << lRtn << std::endl;
    ldap_unbind(pLdapConnection);
    return -1;
  }

  // Connect to the server.
  lRtn=ldap_connect(pLdapConnection, NULL);
  
  if (lRtn != LDAP_SUCCESS) {
    std::cout << "ldap_connect failed with error " << std::hex << lRtn << std::endl;
    ldap_unbind(pLdapConnection);
    return -1;
  }
  
  // Bind with credentials.
  std::cout << "Binddn:" << binddn << std::endl;
  lRtn=ldap_bind_s( pLdapConnection, (PCHAR)(binddn.c_str()), (PCHAR)(password.c_str()), LDAP_AUTH_SIMPLE);
  if (lRtn != LDAP_SUCCESS) {
    std::cout << "ldap_bind_s failed with error " << std::hex << lRtn << std::endl;
    ldap_unbind(pLdapConnection);
    return -1;
  }
 
  ULONG errorCode=LDAP_SUCCESS;
  LDAPMessage* pSearchResult;

  std::cout << "base:" << base << ", filter:" << filter << std::endl;
  errorCode=ldap_search_s(pLdapConnection, (PCHAR)(base.c_str()), LDAP_SCOPE_SUBTREE, (PCHAR)(filter.c_str()), NULL, 0, &pSearchResult);
  
  if (errorCode != LDAP_SUCCESS) {
    std::cout << "ldap_search_s failed with error " << std::hex << errorCode << std::endl;
    ldap_unbind_s(pLdapConnection);
    if (pSearchResult != NULL) ldap_msgfree(pSearchResult);
    return -1;
  }
  
  // Get the number of entries returned.
  ULONG numberOfEntries;
  numberOfEntries=ldap_count_entries( pLdapConnection, pSearchResult);
  
  if (numberOfEntries == 0) {
    std::cout << "ldap_count_entries failed with error " << std::hex << errorCode << std::endl;
    ldap_unbind_s(pLdapConnection);
    if (pSearchResult != NULL) ldap_msgfree(pSearchResult);
    return -1;
  } else std::cout << "ldap_count_entries succeeded" << std::endl;
  
  std::cout << "The number of entries is: " << numberOfEntries << std::endl;;
  
  // Loop through the search entries, get, and output the requested list of attributes and values.
  LDAPMessage* pEntry=NULL;
  //PCHAR pEntryDN=nullptr;
  ULONG iCnt=0;
  char* sMsg;
  BerElement* pBer=NULL;
  PCHAR pAttribute=NULL;
  PCHAR* ppValue=NULL;
  ULONG iValue=0;
  
  for( iCnt=0; iCnt < numberOfEntries; iCnt++ ) {
    // Get the first/next entry.
    if ( !iCnt ) pEntry=ldap_first_entry(pLdapConnection, pSearchResult);
    else pEntry=ldap_next_entry(pLdapConnection, pEntry);
    
    // Output a status message.
    sMsg=(char *)(!iCnt ? "ldap_first_entry" : "ldap_next_entry");
    if ( pEntry == NULL ) {
      std::cout << sMsg << " failed with error " << std::hex << LdapGetLastError() << std::endl;
      ldap_unbind_s(pLdapConnection);
      ldap_msgfree(pSearchResult);
      return -1;
    } else std::cout << sMsg << " succeeded" << std::endl;
    
    // Output the entry number.
    std::cout << "ENTRY NUMBER " << iCnt << std::endl;
        
    // Get the first attribute name.
    pAttribute=ldap_first_attribute( pLdapConnection, pEntry, &pBer);
    
    // Output the attribute names for the current object and output values.
    while(pAttribute != NULL) {
      // Output the attribute name.
      std::cout << " " << pAttribute << " ";
      
      // Get the string values.
      ppValue=ldap_get_values( pLdapConnection, pEntry, pAttribute);

      // Print status if no values are returned (NULL ptr)
      if (ppValue == NULL) {
        std::cout << ": [NO ATTRIBUTE VALUE RETURNED]";
      } else { // Output the attribute values
        iValue=ldap_count_values(ppValue);
        if (!iValue) {
          std::cout << ": [BAD VALUE LIST]";
        } else {
          // Output the first attribute value
          std::cout << ": " << *ppValue;

          // Output more values if available
          ULONG z;
          for(z=1; z<iValue; z++) { std::cout << ", " << ppValue[z];
          }
        }
      } 

      // Free memory.
      if (ppValue != NULL) ldap_value_free(ppValue);
      ppValue=NULL;
      ldap_memfree(pAttribute);
      
      // Get next attribute name.
      pAttribute=ldap_next_attribute( pLdapConnection, pEntry, pBer);
      std::cout << std::endl;
    }
    
    if ( pBer != NULL ) ber_free(pBer,0);
    pBer=NULL;
  }
  
  //----------------------------------------------------------
  // Normal cleanup and exit.
  //----------------------------------------------------------
  ldap_unbind(pLdapConnection);
  ldap_msgfree(pSearchResult);
  ldap_value_free(ppValue);
  return 0;
  
}

int main(int argc, char* argv[])
{
 if (argc == 7) {
   MyLDAPSearch(argv[1], std::stoi(argv[2]), argv[3], argv[4], argv[5], argv[6]);
 } else std::cout << "./myldsrch ldap_server_ip ldap_server_port binddn (includes connection uid) password base filter\nAny of these argument that includes space must be surrounded by quotes or double quotes." << std::endl;
 /*
Exemple : ./myldsrch 172.16.3.94 1403 'uid=adm_sts,ou=Directory Administrators,o=ASPI.fr' 'adm_sts password' 'uid=lalannd2, ou=utilisateur,o=ASPI.fr' '(&(objectclass=persondsi))'
Equivalent avec MSys2 sous Windows10 sur mon PC TAS :
ldapsearch -x -LLL -H ldap://tlldap02.aes.alcatel.fr:1403 -D "uid=adm_sts,ou=Directory Administrators,o=ASPI.fr" -W -b "uid=lalannd2, ou=utilisateur,o=ASPI.fr" '(&(objectclass=persondsi))'
Equivalent sous RedHat7.4 sur tlbefgitp01 :
ldapsearch -x -LLL -h tlldap02 -p 1403 -D "uid=adm_sts,ou=Directory Administrators,o=ASPI.fr" -W -b "uid=lalannd2, ou=utilisateur,o=ASPI.fr" '(&(objectclass=persondsi))'
*/
}

