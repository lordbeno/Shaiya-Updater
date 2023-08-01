#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

#pragma pack(push, 1)
struct SFile
{
    bool assigned{ false };
    bool append{ true };
    std::string path;
    std::string name;
    uint64_t offset;
    uint64_t length;
    std::vector<char> buffer;
};

struct SFolder
{
    std::string name;
    uint32_t fileCount;
    uint32_t folderCount;
    std::vector<SFile> files;
    std::vector<SFolder> folders;

    size_t folders_size() { return folders.size(); }
};

#pragma pack(pop)

namespace update
{
    using namespace System;
    using namespace System::Net;

    const std::string DOWNLOAD_URL = "https://shaiyaspectre.com/Shaiya/patch/";
    const std::string INI_FILE_URL = "https://shaiyaspectre.com/Shaiya/";
    /*
        **FORMAT**

        filter.txt
        PRU.ini
        Character\KillStatus.SData
        Sound\Music\Rain.wav
        world\1.wld
        world\2.txt
    */
    const std::string DELETE_LIST_FILENAME = "delete.txt";

    HttpStatusCode getStatusCode(String^ url);
    int getArchive(int version, int step, int total);
    int open();
    void assign(const std::string& path, SFolder* folder, std::vector<SFile>& patches, bool deleteList);
    void read(const std::string& path, SFolder* folder, std::ifstream* ifs, std::vector<SFile>& patches);
    void seek(std::vector<SFile>& patches);
    void init();
}

namespace sah
{
    inline const char* FILE_SIGNATURE = "fff"; //"SAH";
    constexpr int BYTE_LENGTH = 56;
    constexpr uint32_t FILE_COUNT_KEY = 109;

    void open(std::vector<SFile>& patches, bool deleteList);
    void read(const std::string& path, SFolder* folder, std::ifstream* ifs, std::vector<SFile>& patches, bool deleteList);
    void save(SFolder* folder, const std::vector<SFile>& patches);
    void write(SFolder* folder, std::ofstream* ofs, const std::vector<SFile>& patches, uint32_t& fileCount);
}

namespace saf
{
    void erase(uint64_t offset, uint64_t length);
    void write(SFile& patch);
}

namespace util
{
    void read(std::ifstream* ifs, std::string& str);
    void write(std::ofstream* ofs, size_t count);
    void write(std::ofstream* ofs, const std::string& str);
}

namespace Program
{
    using namespace System;
    using namespace System::Collections;
    using namespace System::ComponentModel;
    using namespace System::Data;
    using namespace System::Diagnostics;
    using namespace System::Drawing;
    using namespace System::IO;
    using namespace System::IO::Compression;
    using namespace System::Threading;
    using namespace System::Windows;
    using namespace System::Windows::Forms;

    public ref class Form1 : public System::Windows::Forms::Form
    {
    private:
        static Program::Form1^ form1;

    public:

        Form1(void)
        {
            InitializeComponent();
            form1 = this;
        }

    protected:

        ~Form1()
        {
            if (components)
            {
                delete components;
            }
        }

    private: System::Windows::Forms::ProgressBar^ progressBar1;
    private: System::Windows::Forms::TextBox^ textBox1;
    private: System::ComponentModel::BackgroundWorker^ backgroundWorker1;
    private: System::Windows::Forms::Button^ button1;
    private: System::Windows::Forms::Button^ button2;
    private: System::Windows::Forms::Button^ button3;
    private: System::Windows::Forms::WebBrowser^ webBrowser1;
    private: System::Windows::Forms::ProgressBar^ progressBar2;

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container^ components;

        #pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->webBrowser1 = (gcnew System::Windows::Forms::WebBrowser());
            this->button2 = (gcnew System::Windows::Forms::Button());
            this->button3 = (gcnew System::Windows::Forms::Button());
            this->progressBar2 = (gcnew System::Windows::Forms::ProgressBar());
            this->SuspendLayout();
            // 
            // progressBar1
            // 
            this->progressBar1->BackColor = System::Drawing::SystemColors::Control;
            this->progressBar1->Location = System::Drawing::Point(12, 469);
            this->progressBar1->Name = L"progressBar1";
            this->progressBar1->Size = System::Drawing::Size(544, 10);
            this->progressBar1->TabIndex = 3;
            // 
            // textBox1
            // 
            this->textBox1->Enabled = false;
            this->textBox1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBox1->ForeColor = System::Drawing::SystemColors::ControlText;
            this->textBox1->Location = System::Drawing::Point(12, 441);
            this->textBox1->Name = L"textBox1";
            this->textBox1->ReadOnly = true;
            this->textBox1->Size = System::Drawing::Size(544, 22);
            this->textBox1->TabIndex = 2;
            this->textBox1->TabStop = false;
            this->textBox1->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // backgroundWorker1
            // 
            this->backgroundWorker1->WorkerReportsProgress = true;
            this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Form1::backgroundWorker1_DoWork);
            // 
            // button1
            // 
            this->button1->BackColor = System::Drawing::SystemColors::Control;
            this->button1->Enabled = false;
            this->button1->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 12.25F, System::Drawing::FontStyle::Bold));
            this->button1->ForeColor = System::Drawing::SystemColors::ControlText;
            this->button1->Location = System::Drawing::Point(562, 441);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(158, 54);
            this->button1->TabIndex = 5;
            this->button1->Text = L"Play";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
            // 
            // webBrowser1
            // 
            this->webBrowser1->AllowWebBrowserDrop = false;
            this->webBrowser1->IsWebBrowserContextMenuEnabled = false;
            this->webBrowser1->Location = System::Drawing::Point(12, 12);
            this->webBrowser1->MinimumSize = System::Drawing::Size(20, 20);
            this->webBrowser1->Name = L"webBrowser1";
            this->webBrowser1->ScriptErrorsSuppressed = true;
            this->webBrowser1->ScrollBarsEnabled = false;
            this->webBrowser1->Size = System::Drawing::Size(708, 420);
            this->webBrowser1->TabIndex = 4;
            this->webBrowser1->TabStop = false;
            this->webBrowser1->Url = (gcnew System::Uri(L"https://shaiyaspectre.com/shaiya/index.php", System::UriKind::Absolute));
            this->webBrowser1->WebBrowserShortcutsEnabled = false;
            // 
            // button2
            // 
            this->button2->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->button2->Location = System::Drawing::Point(138, 405);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(120, 30);
            this->button2->TabIndex = 0;
            this->button2->TabStop = false;
            this->button2->Text = L"Discord";
            this->button2->UseVisualStyleBackColor = true;
            this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
            // 
            // button3
            // 
            this->button3->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->button3->Location = System::Drawing::Point(12, 405);
            this->button3->Name = L"button3";
            this->button3->Size = System::Drawing::Size(120, 30);
            this->button3->TabIndex = 1;
            this->button3->TabStop = false;
            this->button3->Text = L"Register";
            this->button3->UseVisualStyleBackColor = true;
            this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
            // 
            // progressBar2
            // 
            this->progressBar2->Location = System::Drawing::Point(12, 485);
            this->progressBar2->Name = L"progressBar2";
            this->progressBar2->Size = System::Drawing::Size(544, 10);
            this->progressBar2->TabIndex = 4;
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackColor = System::Drawing::Color::Black;
            this->ClientSize = System::Drawing::Size(732, 504);
            this->Controls->Add(this->progressBar2);
            this->Controls->Add(this->button3);
            this->Controls->Add(this->button2);
            this->Controls->Add(this->webBrowser1);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->textBox1);
            this->Controls->Add(this->progressBar1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Fixed3D;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"Form1";
            this->ShowIcon = false;
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"Shaiya Spectre Updater";
            this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
            this->ResumeLayout(false);
            this->PerformLayout();

        }
        #pragma endregion

        public: static void performStep(int progressBarId)
        {
            switch (progressBarId)
            {
            case 1:
                form1->progressBar1->PerformStep();
                break;
            case 2:
                form1->progressBar2->PerformStep();
                break;
            default:
                break;
            }
        }

       public: static void setProgress(int progressBarId, int value, int maximum, int step)
       {
           switch (progressBarId)
           {
           case 1:
               form1->progressBar1->Value = value;
               form1->progressBar1->Maximum = maximum;
               form1->progressBar1->Step = step;
               // If the value is equal to the maximum, set the value to the maximum
               if (value == maximum) {
                   form1->progressBar1->Value = maximum;
               }
               break;
           case 2:
               form1->progressBar2->Value = value;
               form1->progressBar2->Maximum = maximum;
               form1->progressBar2->Step = step;
               // If the value is equal to the maximum, set the value to the maximum
               if (value == maximum) {
                   form1->progressBar2->Value = maximum;
               }
               break;
           default:
               break;
           }
       }

   public: static int getProgressBarMaximum(int progressBarId)
             {
                 switch (progressBarId)
                 {
                 case 1:
                     return form1->progressBar1->Maximum;
                 case 2:
                     return form1->progressBar2->Maximum;
                 default:
                     return 0;
                 }
             }

        public: static void setTextBox1(const std::string& text)
        {
            form1->textBox1->Text = gcnew String(text.c_str());
        }

        public: static void setTextBox1(String^ text)
        {
            form1->textBox1->Text = text;
        }

        private: System::Void Form1_Load(System::Object^ sender, System::EventArgs^ e)
        {
            if (FindWindowExA(nullptr, nullptr, "GAME", nullptr))
            {
                Application::Exit();
            }

            webBrowser1->SendToBack();
            backgroundWorker1->RunWorkerAsync();
        }

        // This event handler is where the time-consuming work is done.
        private: System::Void backgroundWorker1_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e)
        {		
            try
            {
                update::init();
            }
            catch (const std::exception& ex)
            {
                textBox1->Text = gcnew String(ex.what());
            }
            catch (Exception^ ex)
            {
                textBox1->Text = ex->Message;
            }

            button1->Enabled = true;
        }
    
        private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) 
        {
            try
            {
                if (std::filesystem::exists("new_updater.exe"))
                {
                    Process::Start("game.exe", "new updater");
                    Application::Exit();
                }
                else
                {
                    Process::Start("game.exe", "start game");
                    Application::Exit();
                }
            }
            catch (Exception^ ex)
            {
                textBox1->Text = ex->Message;
                button1->Enabled = false;
            }
        }

        private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) 
        {
            Process::Start("https://discord.gg/g2egEYJS5y"); //discord 
        }

        private: System::Void button3_Click(System::Object^ sender, System::EventArgs^ e) 
        {
            Process::Start("https://shaiyaspectre.com/register"); //register 
        }
    };
}
