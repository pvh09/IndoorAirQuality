using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Configuration;
using System.IO.Pipes;
using System.Diagnostics.Eventing.Reader;

namespace Giaodien_Quanly_Vuon
{

    public class LogFile
    {
        private Object locker = new object();
        private static LogFile _Logger;
        private StreamWriter logWriter;
        private Action asyncFileCreator;
        private string logFile;
        private bool lockTheStream;
        string filePath = System.Configuration.ConfigurationManager.AppSettings["Path"];
        FileStream objFilestream;
        public LogFile()
        {

            logFile = filePath + "logs\\" + DateTime.Now.ToString("yyyyMMdd") + ".log";
            //objFilestream = new FileStream(string.Format(logFile), FileMode.Append, FileAccess.Write);
            CreateLogFile(logFile);
            //logWriter = new StreamWriter(objFilestream);
            asyncFileCreator = CreateNextLogfile;
            asyncFileCreator.BeginInvoke(null, null);
        }

        public enum LogKind{
            Error,
            Information,
            GetData
    }

    /// <summary>
    /// Creates the physical log file. 
    /// </sum
    /// mary>
    private void CreateLogFile(String logFile)
        {
            if (!File.Exists(logFile))
            {
                using (StreamWriter sw = File.CreateText(logFile))
                {
                }
                File.Create(logFile).Close();
            }
        }

        /// <summary>
        /// This method create a log file for next day exactly 5 second before 12am. 
        /// The next day, it redirects the logs to new file
        /// This method runs constantly on a background thread.
        /// </summary>
        private void CreateNextLogfile()
        {
            while (true)
            {
                DateTime currentDtTm = DateTime.Now;
                while (!(currentDtTm.Hour >= 23 && currentDtTm.Minute >= 59 && currentDtTm.Second >= 55))
                {
                    System.Threading.Thread.Sleep(1000);
                    currentDtTm = DateTime.Now;
                }

                logFile = filePath + "logs\\" + DateTime.Now.ToString("yyyyMMdd") + ".log";
                CreateLogFile(logFile);
                lockTheStream = true;

                //wait till day changes
                while (currentDtTm.AddDays(1).Day != DateTime.Now.Day) ;

                lock (logWriter)
                {
                    logWriter.Dispose();
                    logWriter = new StreamWriter(File.OpenWrite(logFile));
                }

                lockTheStream = false;

            }
        }

        /// <summary>
        /// A helper method to create singleton instance of Logger class
        /// </summary>
        /// <returns></returns>
        public LogFile GetLogger()
        {
            if (_Logger == null)
            {
                lock (locker)
                {
                    if (_Logger == null)
                    {
                        _Logger = new LogFile();
                    }
                }
            }

            return _Logger;
        }

        /// <summary>
        /// Write the log
        /// </summary>
        /// <param name="logMessage">Log message to be written to log file</param>
        public void WriteLog(LogKind kind, string logMessage)
        {
            try  
            {
                using (StreamWriter logWriter = File.AppendText(logFile))
                {
                    logWriter.WriteLine(DateTime.Now.ToString() + "--" + kind + "--" + logMessage);
                }
            }  
            catch(Exception ex)  
            {
                Console.WriteLine(ex.Message);
            }
        }

    }
}
