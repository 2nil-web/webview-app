#!/bin/bash

curl -sku 'lalannd2:ocvdBum12$*4' 'https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-11-14&limit=1000' | jq
# <==>
./rest.exe -r 'https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-11-14&limit=1000' | jq

