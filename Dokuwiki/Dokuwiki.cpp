#include <iostream>
#include <string>
#include <set>
#include <map>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <io.h> 
#include <fcntl.h> 
#include <regex> 
#include <sstream>
#include <fstream>
#include <codecvt>

using namespace std;

// wcout, cout fonksiyonuyla ayni islevdedir, turkce karakterleri kaybetmemek icin string yerine wide stringlerle calistik
// widestringlerle calistigimizda normal string fonksiyonlarini kullanamayiz, bunun icin widestringe uygun fonksiyonlar kullanilmalidir
// ornegin wsting, wcout, wcin, wregex vb.

const filesystem::path workDir(L".\\Üniversite");

// dosyaya yazma ve ya dosyadan okuma islemleri icin gerekli iki adet fonksiyon
// okunan utf8 verisinin turkce karakterler dahil wide stringe donusturulmesini yapar
std::wstring ConvertUtf8ToWide(const std::string& str)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
    return wstr;
}

// dosyaya wide character yazarken utf8e donusturmek icin gerekli
std::string ConvertWideToUtf8(const std::wstring& wstr)
{
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

// bir string icindeki eslesen tum stringleri degistirme fonksiyonu, etiket isimlerini degistirirken kullaniyoruz
wstring ReplaceAll(wstring str, const std::wstring& from, const std::wstring& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
        str.replace(start_pos, from.length(), to);
    }
    return str;
}

// dosyalar uzerinde normal sekilde kelime arama
void KelimeArama(wstring kelime)
{
    // altklasorler uzerinde dolasmak icin gerekli iterate for loopu
    for (const auto& dirEntry : filesystem::recursive_directory_iterator(workDir))
    {
        if (dirEntry.path().extension() == ".txt")
        {
            // dosyayi aciyoruz
            ifstream inFile(dirEntry.path());

            unsigned int curLine = 0;

            string line;

            // dosyada satir satir okuma
            while (getline(inFile, line))
            {
                curLine++;
                // dosyadan okunan utf8i widestringe cevirelim, regex aramalari icin gerekli
                wstring wline= ConvertUtf8ToWide(line);

                // kelime kelime parse edip sadece uygun kelimeyi ayiracak regexi olusturalim
                wregex etiketRegex(L"(?:^|\\W)" + kelime + L"(?:$|\\W)");

                wsregex_iterator it(wline.begin(), wline.end(), etiketRegex);
                for (; it != wsregex_iterator(); ++it) {
                    // bulunan her kelime icin
                    wcout << "Bulundu:: Dosya: " << dirEntry.path() << " Satir: " << curLine << endl;
                }
            }
            inFile.close();
        }
    }
}

// etkiet aramada sadece gelen etiket isminin basina ve sonuna koseli parantezler ekleyerek kelime aramasi yapiyoruz
void EtiketArama(wstring etiket)
{
    int count = 0;

    for (int i = 0; i < etiket.size(); i++)
        if (etiket[i] == ' ') count++;

    if (count > 1) return;
	
    etiket = L"\\[\\[" + etiket + L"\\]\\]";
    KelimeArama(etiket);
}

void EtiketleriListele()
{
    // txt dosya isimlerini ve bu dosyalardan okunan etiket isimlerini kaydetmek icin kume kullandik
    set<wstring> txtListesi;
    set<wstring> etiketListesi;

    // etiket etiket ayirarak koseli parantezler icinde en fazla iki kelime olacak sekilde arama yapicak regexi olusturuyoruz
    wregex etiketRegex(L"\\[\\[([A-Za-z_ÖöÇçŞşİiĞğÜü]+)( [A-Za-z_ÖöÇçŞşİiĞğÜü]+){0,1}\\]\\]"); // koseli parantezler arasinda bir ve ya iki kelime aramasi icin regex
    wcout << L"Etiket Taraması" << endl;

    for (const auto& dirEntry : filesystem::recursive_directory_iterator(workDir))
    {
        if (dirEntry.path().extension() == ".txt")
        {
            // dosya isminden uzanti kismini cikartiyoruz
            size_t lastindex = dirEntry.path().filename().u8string().find_last_of(".");
            wstring rawname = ConvertUtf8ToWide(dirEntry.path().filename().u8string().substr(0, lastindex));
            // uzantisiz kismi txtlistesine ekliyoruz
            txtListesi.insert(rawname);

            wcout << L"File: " << dirEntry.path() << endl;

            ifstream inFile(dirEntry.path());

            string line;
            while (getline(inFile, line))
            {
                // okunan verideki turkce karkterleri kaybetmemek icin widestringe cevirelim
                wstring wline = ConvertUtf8ToWide(line);

                // olusturdugumuz regexi satirda arayalim
                wsregex_iterator it(wline.begin(), wline.end(), etiketRegex);

                for (; it != wsregex_iterator(); ++it) {
                    // bulunan etiketi etiket listesine ekleyelim
                    wstring etiketIsmi = (*it)[0].str();
                    etiketIsmi = etiketIsmi.substr(2, etiketIsmi.size() - 4);
                    etiketListesi.insert(etiketIsmi);
                    wcout << L"\t" << etiketIsmi << endl;
                }
            }
            inFile.close();
        }
    }

    // yetim etiketleri ekrana yazdiralim
    wcout << "Yetim Etiketler:" << endl;

    for (auto& etiket : etiketListesi)
    {
        // etiket ismindeki bosluklari alttire ile degistirelim
        wstring etiketIsmi = ReplaceAll(etiket, L" ", L"_");

        // bu etiket txt listesinde yok ise yetim etikettir, ekrana yazdiralim
        if (find(txtListesi.begin(), txtListesi.end(), etiketIsmi) == txtListesi.end())
        {
            wcout << L"\t" << etiket << endl;
        }
    }

    // istenen etiketleri ekrana yazdiralim
    wcout << L"İstenen Etiketler:" << endl;

    for (auto& txt : txtListesi)
    {
        // txt ismindeki alttireleri bosluk ile degistirelim
        wstring txtIsmi = ReplaceAll(txt, L"_", L" ");

        // bu txt ismi etiket listesinde yok ise istenen etikettir, ekrana yazdiralim
        if (find(etiketListesi.begin(), etiketListesi.end(), txtIsmi) == etiketListesi.end())
        {
            wcout << L"\t" << txt << endl;
        }
    }
}

// txt belgesini alt klasorlerde ariyor, bulduktan sonra eski etiketler eslesen butun etiketleri yenileriyle degisitiriyor.
// txt belgesinin ismi de eski etiket ismi ile eslesiyorsa yeni etiket ismi ile degistiriyor.
void EtiketAdiGuncelle(wstring txtIsmi, wstring eskiEtiketAdi, wstring yeniEtiketAdi)
{
    wstring peskiEtiketAdi = eskiEtiketAdi;
    wstring pyeniEtiketAdi = yeniEtiketAdi;

    // eski ve yeni etiket isimlerinin baslarina koseli parantezler ekliyoruz
    eskiEtiketAdi = L"[[" + eskiEtiketAdi + L"]]";
    yeniEtiketAdi = L"[[" + yeniEtiketAdi + L"]]";
    // altklasorler uzerinde dolasmak icin gerekli iterate for loopu
    for (const auto& dirEntry : filesystem::recursive_directory_iterator(workDir))
    {
        if (dirEntry.path().extension() == ".txt")
        {
            // uzantiyi kaldirip sadece oysa ismini alıyor
            size_t lastindex = dirEntry.path().filename().u8string().find_last_of(".");
            wstring rawname = ConvertUtf8ToWide(dirEntry.path().filename().u8string().substr(0, lastindex));

            // verilen txt ismiyle karsilastir, eger ayni ise dosya blunmus demektir
            if (rawname.compare(txtIsmi) == 0)
            {
                // dosya icerigi desgitirilecegi icin tum dosya icerigini desigtirdikce yeni bir vectore al
                std::vector<wstring> lines;

                ifstream inFile(dirEntry.path());

                string line;
                while (getline(inFile, line))
                {
                    // okunan utf8 verisini widestringe cevir
                    wstring wline = ConvertUtf8ToWide(line);

                    // satiurdaki etiket eslesmelerinin hepsini yenisiyle degistir
                    wstring replaced = ReplaceAll(wline, eskiEtiketAdi, yeniEtiketAdi);

                    // degistirilen satiri vectore ekle
                    lines.push_back(replaced);
                }
                inFile.close();

                // dosyayi output olarak tekrar ac
                ofstream outFile(dirEntry.path());

                // vectordeki tum satirlar icin
                for (auto& line : lines)
                {
                    // satiri tekrar utf8e cevir, dosyaya yaz
                    outFile << ConvertWideToUtf8(line) << endl;
                }
                outFile.close();

                // dosya ismindeki degisiklikleri yap ve yeniden adlandir
                peskiEtiketAdi = ReplaceAll(peskiEtiketAdi, L" ", L"_");
                pyeniEtiketAdi = ReplaceAll(pyeniEtiketAdi, L" ", L"_");
                wstring newFileName = ReplaceAll(dirEntry.path(), peskiEtiketAdi, pyeniEtiketAdi);//txt dosyasinin isminin degistirilmesi
                //filesystem::rename(dirEntry.path(), newFileName);
                wstring oldFileName = ConvertUtf8ToWide(dirEntry.path().u8string());
                int res = _wrename(oldFileName.c_str(), newFileName.c_str());
                break;
            }
        }
    }
}

// cikti doyasi olusturup verileri icine yazar
void CiktiOlustur()
{
    // txt isimlerini tutmak icin bir set kullandik
    set<wstring> txtListesi;
    // txt belgelerindeki etiket isimlerini ve kac kere kullanildiklarini tutmak icin bir map kullandik
    map<wstring, uint32_t> etiketListesi;

    // etike regexi
    wregex etiketRegex(L"\\[\\[([A-Za-z_ÖöÇçŞşİiĞğÜü]+)( [A-Za-z_ÖöÇçŞşİiĞğÜü]+){0,1}\\]\\]"); // koseli parantezler arasinda bir ve ya iki kelime aramasi icin regex

    for (const auto& dirEntry : filesystem::recursive_directory_iterator(workDir))
    {
        if (dirEntry.path().extension() == ".txt")
        {
            size_t lastindex = dirEntry.path().filename().u8string().find_last_of(".");
            wstring rawname = ConvertUtf8ToWide(dirEntry.path().filename().u8string().substr(0, lastindex));
            // uzantisiz kismi txtlistesine ekliyoruz
            txtListesi.insert(rawname);

            ifstream inFile(dirEntry.path());

            string line;

            while (getline(inFile, line))
            {
                wstring wline = ConvertUtf8ToWide(line);

                // satir uzerinde etiket regexini arat
                wsregex_iterator it(wline.begin(), wline.end(), etiketRegex);

                // bulunan her etiket icin
                for (; it != wsregex_iterator(); ++it) {
                    wstring etiketIsmi = (*it)[0].str();
                    etiketIsmi = etiketIsmi.substr(2, etiketIsmi.size() - 4);

                    // eger etiket ismi zaten map uzerinde var ise, sayisini bir arttir, yoksa yeni kayit olarak ekle
                    const auto& it2 = etiketListesi.find(etiketIsmi);
                    if (it2 == etiketListesi.end())
                    {
                        etiketListesi.insert({ etiketIsmi, 1 });
                    }
                    else
                    {
                        it2->second++;
                    }
                }
            }
            inFile.close();
        }
    }

    // cikti dosyasini olustur
    ofstream outFile(workDir / "output.txt");

    outFile << left << setw(30) << "Etiket Listesi" << ConvertWideToUtf8(L"Tekrar Sayısı") << endl;

    for (auto& etiket : etiketListesi)
    {
        // etiket ismini ve erisim sayisini duzenli sekilde dosyaya yaz
        outFile << ConvertWideToUtf8(etiket.first) << string(30 - etiket.first.length(), ' ') << etiket.second << endl;
    }

    outFile << endl;

    outFile << "Yetim Etiketler:" << endl;

    for (auto& etiket : etiketListesi)
    {
        // etiket isminden txt ismine gecis icin gerekli degisiklikligi yap
        wstring etiketIsmi = ReplaceAll(etiket.first, L" ", L"_");

        // etiket ismi txt listesinde bulunmadiysa yetim etikettir
        if (find(txtListesi.begin(), txtListesi.end(), etiketIsmi) == txtListesi.end())
        {
            outFile << ConvertWideToUtf8(etiket.first) << endl;
        }
    }
    outFile.close();
}

int main()
{
    // turk karakter giris cikisi icin geerkli fonksiyonlar
    setlocale(LC_ALL, "Turkish");
    SetConsoleCP(1252);
    SetConsoleOutputCP(1252);

    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    int secim = 1;

    while (secim)
    {
        //wcout << L"<ÖöÇçŞşİiĞğÜü>" << endl;
        wcout << "Ana menu" << endl;
        wcout << "1- Arama" << endl;
        wcout << "2- Guncelleme" << endl;
        wcout << "3- Dosyaya Yazma" << endl;
        wcout << "0- Cikis" << endl;

        wcin >> secim;

        switch (secim)
        {
            case 1:
            {
                int secim2;
                wcout << "Arama menusu" << endl;
                wcout << "1- Kelime" << endl;
                wcout << "2- Etiket" << endl;
                wcout << "3- Yetim/Istenen Etiketleri Listele" << endl;
                wcout << "(Diger)- Iptal" << endl;
                wcin >> secim2;

                switch (secim2)
                {
                case 1:
                {
                    wstring kelime;
                    wcout << "Aranacak kelimeyi giriniz: ";
                    getline(wcin >> ws, kelime);
                    KelimeArama(kelime);
                    break;
                }
                case 2:
                {
                    wstring etiket;
                    wcout << "Aranacak etiketi giriniz: ";
                    getline(wcin >> ws, etiket);
                    EtiketArama(etiket);
                    break;
                }
                case 3:
                {
                    EtiketleriListele();
                    break;
                }
                }
                break;
            }
            case 2:
            {
                wstring dosyaAdi, eskiEtiketAdi, yeniEtiketAdi;
                wcout << "Dosya giriniz: ";
                getline(wcin >> ws, dosyaAdi);
                wcout << "Eski etiket adini giriniz: ";
                getline(wcin >> ws, eskiEtiketAdi);
                wcout << "Yeni etiket adini giriniz: ";
                getline(wcin >> ws, yeniEtiketAdi);
                EtiketAdiGuncelle(dosyaAdi, eskiEtiketAdi, yeniEtiketAdi);
                break;
            }
            case 3:
            {
                CiktiOlustur();
                break;
            }
        }

        system("pause");
    }

    return 0;
}
