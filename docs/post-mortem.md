## Varför valde ni att göra det specialiseringsprojektet som ni valde?
_(Vad gör er intresserade av detta ämnet? Vilka mål hade ni när ni började arbeta? Hur tänkte ni på scopet på arbetet?)_

Jag har i princip vuxit upp med first person shooters och fann 3kliksphilips video om asynchronous reprojection och comrade stinger's demo väldigt intressanta. Under hela min uppväxt har jag varit begränsad av att spela på en potatisrigg med mycket framerate-problem, och asynchronous reprojection är en potentiell lösning eller vartfall ett hjälpmedel.

När jag började arbetet hade jag som huvudmål att iallafall skapa en grundläggande renderare i Direct3D 12 och bli bekant med API:n. Jag var orolig i början (och är fortfarande till viss del) att jag skulle stöta på hårdvarobegränsningar jag inte skulle kunna jobba mig förbi. Att virtual reality-headset ofta har flera grafikkort, att Direct3D 11 inte har stöd för concurrent drawcalls och att ingen information alls finns på nätet om hur man implementerar asynchronous reprojection gjorde framtidsutsikterna grumliga.

Jag visste att om jag ville lyckas behövde jag lära mig Direct3D 12, men jag hade bara en vag gissning hur komplext det skulle bli i förhållande till Direct3D 11. Min planering reflekterade den vagheten då jag inte hade en aning om vad för koncept innan jag hade möjligheten att skapa en renderare från scratch.
## Hur fungerade er tidsplanering?
_(Lyckades ni dela upp projektet i dagsblock? Var ni i fas under arbetet? Var er planering väldigt grov, eller behövde ni microplanera varje dag? Hur påverkade tidsplaneringen ert arbete? Vad skulle ni gjort annorlunda med planeringen? Hur väl lyckades ni estimera tiden?)_

I planeringen listade jag endast draw triangle, draw cubic demo world, GPU preemption och async reprojection i stora block som täckte veckor med undantag från onsdagar som dedikerades åt hemsidan. Jag har upplevt att tidsestimeringen varit omöjlig att göra när man inte känner till existensen av essentiella koncept som root signatures, pipeline state objects, command allocators, command lists, command queues, fences, upload heaps, default heaps, resource barriers med mera och har försökt maximera tiden i arbete.

Jag räknade inte med tid som skulle försvinna från gästföreläsningar, personligt brev, cv/resume och föreläsningar i nätverksprogrammering. Att jag skapade hemsidan från scratch i [Hugo](https://gohugo.io/) gjorde det tidskrävande att finjustera hemsidan även om jag hade viss erfarenhet av html och css sedan innan. Hade jag inte velat jobba med hemsidan på fritiden skulle jag nog behöva dubbla tiden dedikerad för hemsidan om jag inte ville använda en webbsidsbyggare.

Jag tror inte jag hade gjort planeringen annorlunda om jag skulle göra om exakt samma specialisering med exakt samma hemsida. Jag tror jag hade behövt välja en annan specialisering och hemsida med mindre scope och därav anpassa min arbetsbörda istället.
## Hur har ni tänkt när ni har byggt er portfolio?
_(Har ni en spotlight som ni riktar tittaren mot? Har ni ett eller flera specifika företag som ni riktar portfolion mot? Varför har ni valt att ha med de saker som ni har med? Hur presenterar ni er själva? Hur presenterar ni era skills?)_

Mina designmål med hemsidan har varit
* det första man ser är att jag heter Ivar Sidorsson och är grafikprogrammerare
* det andra man ser är en länk till nedladdningsbart CV (kanske borde göra tydligare att det är ett cv)
* minimera bloat från navbars
* minimera noise
* overview över portfolio och skills utan att navigera på mindre sidor
* läsa individuellt portfolio utan noise
* responsiv design för telefon
* inga third party dependencies som kan paja
* kunna lägga till projekt i mitt portfolio utan att behöva röra html, css och mixtra med layout

Mina designmål med cv har varit
* Första man ser är mitt namn och kontaktuppgifter
* Andra man ser är att jag är en grafikprogrammerare
* Tredje man ser är mina skickligheter, med ärlig kort beskrivning
* Ha inga summaries eller personliga saker
* Ha med alla personliga länkar, man ska inte behöva klicka in på min hemsida för att hitta något
* Namnge CV 'Ivar Sidorsson Resume.pdf' så att det kan paras med 'Ivar Sidorsson Cover Letter.pdf', båda ska alltid bifogas i ansökningar även om cover letter copy-pastas i ett textfält
* Ha så simpel formattering som möjligt för att underlätta vidareutveckling

Jag är relativt nöjd med hemsidan och har lärt mig väldigt mycket om Hugo och styling/layout på hemsidor även om denna hemsidan är rätt minimalistisk. Jag känner definitivt att det är en portfoliosida jag är villig att hålla vid liv givet att den enda formatteringen jag behöver för att lägga till portfolio är ett markdown-dokument. Den är hostad gratis via GitHub pages och laddas upp via GitHub Actions så fort jag pushar en förändring, och det enda jag betalar för är min egna domän.

Alla projekt listas i hemsidan efter kronologisk ordning, så de senare som min specialisering och spelprojekt 8 kommer synas först. De sorteras automatiskt efter en tidsstämpel som sätts i markdown-dokumentens front matter.