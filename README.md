# Http-Badge-Reader-for-arduino

Arduino file for reading badge with RC522 and send online GET request for validating access (online mode)


### Funzionamento 

Lo script, tramite lo shield ethernet, effettuerà una richiesta al server http://[your-server]/track?card=[ID-card] per ogni badge letto.
Il server dovrà rispondere "RESULT:3" oppure "RESULT:0" dove il numero indica quanti accessi rimangono. Se sarà magggiore di zero verrà abilitato il relè


### Hardware

https://www.thingiverse.com/thing:3058990
