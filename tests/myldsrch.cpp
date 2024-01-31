//----------------------------------------------
// Performing an LDAP Synchronous Search.
//
// Be aware that you must set the command prompt screen buffer 
// height to 350 and the width to 90.
//-------------------------------------------------------------

#include <windows.h>
#include <winldap.h>
#include <winber.h>
#include <rpc.h>
#include <rpcdce.h>
#include <stdio.h>

//-----------------------------------------------------------
// This subroutine must have validated credentials (name and
// password) passed to it.
//-----------------------------------------------------------
int MyLDAPSearch(PCHAR pUserName, PCHAR pPassword)
{
    //---------------------------------------------------------
    // Initialize a session. LDAP_PORT is the default port, 389.
    //---------------------------------------------------------
    PCHAR hostName = (PCHAR )"fabrikam.com";
    LDAP* pLdapConnection = NULL;
    
    pLdapConnection = ldap_init(hostName, LDAP_PORT);
    
    if (pLdapConnection == NULL)
    {
        printf("ldap_init failed with 0x%x.\n",LdapGetLastError());
        ldap_unbind(pLdapConnection);
        return -1;
    }
    else
        printf("ldap_init succeeded \n");
    
    
    //-------------------------------------------------------
    // Set session options.
    //-------------------------------------------------------
    ULONG version = LDAP_VERSION3;
    ULONG numReturns = 10;
    ULONG lRtn = 0;
    
    // Set the version to 3.0 (default is 2.0).
    lRtn = ldap_set_option(
                    pLdapConnection,           // Session handle
                    LDAP_OPT_PROTOCOL_VERSION, // Option
                    (void*) &version);         // Option value

    if(lRtn == LDAP_SUCCESS)
        printf("ldap version set to 3.0 \n");
    else
    {
        printf("SetOption Error:%0lX\n", lRtn);
        ldap_unbind(pLdapConnection);
        return -1;
    }

    // Set the limit on the number of entries returned to 10.
    lRtn = ldap_set_option(
                    pLdapConnection,       // Session handle
                    LDAP_OPT_SIZELIMIT,    // Option
                    (void*) &numReturns);  // Option value

    if(lRtn == LDAP_SUCCESS)
        printf("Max return entries set to 10 \n");
    else
    {
        printf("SetOption Error:%0lX\n", lRtn);
        ldap_unbind(pLdapConnection);
        return -1;
    }
    
    
    //--------------------------------------------------------
    // Connect to the server.
    //--------------------------------------------------------
    
    lRtn = ldap_connect(pLdapConnection, NULL);
    
    if(lRtn == LDAP_SUCCESS)
        printf("ldap_connect succeeded \n");
    else
    {
        printf("ldap_connect failed with 0x%lx.\n",lRtn);
        ldap_unbind(pLdapConnection);
        return -1;
    }
    
    
    //--------------------------------------------------------
    // Bind with credentials.
    //--------------------------------------------------------
    PCHAR pMyDN = "DC=fabrikam,DC=com";
    SEC_WINNT_AUTH_IDENTITY secIdent;
 
    secIdent.User = (unsigned char*)pUserName;
    secIdent.UserLength = strlen(pUserName);
    secIdent.Password = (unsigned char*)pPassword;
    secIdent.PasswordLength = strlen(pPassword);
    secIdent.Domain = (unsigned char*)hostName;
    secIdent.DomainLength = strlen(hostName);
    secIdent.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
    
    lRtn = ldap_bind_s(
                pLdapConnection,      // Session Handle
                pMyDN,                // Domain DN
                (PCHAR)&secIdent,     // Credential structure
                LDAP_AUTH_NEGOTIATE); // Auth mode
    if(lRtn == LDAP_SUCCESS)
    {
        printf("ldap_bind_s succeeded \n");
        secIdent.Password = NULL; // Remove password pointer
        pPassword = NULL;         // Remove password pointer
    }
    else
    {
        printf("ldap_bind_s failed with 0x%lx.\n",lRtn);
        ldap_unbind(pLdapConnection);
        return -1;
    }
  
    //----------------------------------------------------------
    // Perform a synchronous search of fabrikam.com for 
    // all user objects that have a "person" category.
    //----------------------------------------------------------
    ULONG errorCode = LDAP_SUCCESS;
    LDAPMessage* pSearchResult;
    PCHAR pMyFilter = "(&(objectCategory=person)(objectClass=user))";
    PCHAR pMyAttributes[6];

    pMyAttributes[0] = "cn";
    pMyAttributes[1] = "company";
    pMyAttributes[2] = "department";
    pMyAttributes[3] = "telephoneNumber";
    pMyAttributes[4] = "memberOf";
    pMyAttributes[5] = NULL;
    
    errorCode = ldap_search_s(
                    pLdapConnection,    // Session handle
                    pMyDN,              // DN to start search
                    LDAP_SCOPE_SUBTREE, // Scope
                    pMyFilter,          // Filter
                    pMyAttributes,      // Retrieve list of attributes
                    0,                  // Get both attributes and values
                    &pSearchResult);    // [out] Search results
    
    if (errorCode != LDAP_SUCCESS)
    {
        printf("ldap_search_s failed with 0x%0lx \n",errorCode);
        ldap_unbind_s(pLdapConnection);
        if(pSearchResult != NULL)
            ldap_msgfree(pSearchResult);
        return -1;
    }
    else
        printf("ldap_search succeeded \n");
    
    //----------------------------------------------------------
    // Get the number of entries returned.
    //----------------------------------------------------------
    ULONG numberOfEntries;
    
    numberOfEntries = ldap_count_entries(
                        pLdapConnection,    // Session handle
                        pSearchResult);     // Search result
    
    if(numberOfEntries == NULL)
    {
        printf("ldap_count_entries failed with 0x%0lx \n",errorCode);
        ldap_unbind_s(pLdapConnection);
        if(pSearchResult != NULL)
            ldap_msgfree(pSearchResult);
        return -1;
    }
    else
        printf("ldap_count_entries succeeded \n");
    
    printf("The number of entries is: %d \n", numberOfEntries);
    
    
    //----------------------------------------------------------
    // Loop through the search entries, get, and output the
    // requested list of attributes and values.
    //----------------------------------------------------------
    LDAPMessage* pEntry = NULL;
    PCHAR pEntryDN = NULL;
    ULONG iCnt = 0;
    char* sMsg;
    BerElement* pBer = NULL;
    PCHAR pAttribute = NULL;
    PCHAR* ppValue = NULL;
    ULONG iValue = 0;
    
    for( iCnt=0; iCnt < numberOfEntries; iCnt++ )
    {
        // Get the first/next entry.
        if( !iCnt )
            pEntry = ldap_first_entry(pLdapConnection, pSearchResult);
        else
            pEntry = ldap_next_entry(pLdapConnection, pEntry);
        
        // Output a status message.
        sMsg = (char *)(!iCnt ? "ldap_first_entry" : "ldap_next_entry");
        if( pEntry == NULL )
        {
            printf("%s failed with 0x%0lx \n", sMsg, LdapGetLastError());
            ldap_unbind_s(pLdapConnection);
            ldap_msgfree(pSearchResult);
            return -1;
        }
        else
            printf("%s succeeded\n",sMsg);
        
        // Output the entry number.
        printf("ENTRY NUMBER %i \n", iCnt);
                
        // Get the first attribute name.
        pAttribute = ldap_first_attribute(
                      pLdapConnection,   // Session handle
                      pEntry,            // Current entry
                      &pBer);            // [out] Current BerElement
        
        // Output the attribute names for the current object
        // and output values.
        while(pAttribute != NULL)
        {
            // Output the attribute name.
            printf("     ATTR: %s",pAttribute);
            
            // Get the string values.

            ppValue = ldap_get_values(
                          pLdapConnection,  // Session Handle
                          pEntry,           // Current entry
                          pAttribute);      // Current attribute

            // Print status if no values are returned (NULL ptr)
            if(ppValue == NULL)
            {
                printf(": [NO ATTRIBUTE VALUE RETURNED]");
            }

            // Output the attribute values
            else
            {
                iValue = ldap_count_values(ppValue);
                if(!iValue)
                {
                    printf(": [BAD VALUE LIST]");
                }
                else
                {
                    // Output the first attribute value
                    printf(": %s", *ppValue);

                    // Output more values if available
                    ULONG z;
                    for(z=1; z<iValue; z++)
                    {
                        printf(", %s", ppValue[z]);
                    }
                }
            } 

            // Free memory.
            if(ppValue != NULL)  
                ldap_value_free(ppValue);
            ppValue = NULL;
            ldap_memfree(pAttribute);
            
            // Get next attribute name.
            pAttribute = ldap_next_attribute(
                            pLdapConnection,   // Session Handle
                            pEntry,            // Current entry
                            pBer);             // Current BerElement
            printf("\n");
        }
        
        if( pBer != NULL )
            ber_free(pBer,0);
        pBer = NULL;
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

}

