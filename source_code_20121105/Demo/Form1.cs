using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Threading;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using System.Reflection.Emit;

namespace HelloWorld_V4
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            foreach (Control ctrl in this.Controls)
            {
                if (ctrl is Button)
                    ctrl.Enabled = false;
            }
            txtOutput.Text = @"Injection is being initialized for CLR/JIT...
This may take a short while because it is searching the address from PDB symbol file.";
            InjectionHelper.Initialize();

            Thread thread = new Thread(WaitForInitialization);
            thread.Start();
        }

        private void WaitForInitialization()
        {
            InjectionHelper.Status status = InjectionHelper.WaitForIntializationCompletion();
            InitializationCompletedDelegate del = new InitializationCompletedDelegate(InitializationCompleted);
            this.BeginInvoke(del, status);

            // the following code collects the offset cache to help me improve the code
            if (status == InjectionHelper.Status.Ready)
            {
                string dir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase);
                dir = Regex.Replace(dir, @"^(file\:\\)", string.Empty);
                dir = Path.Combine(dir, "cache");

                if (Directory.Exists(dir))
                {                
                    string [] files = Directory.GetFiles(dir, "*.cache", SearchOption.TopDirectoryOnly);
                    foreach (string file in files)
                    {
                        try
                        {
                            // collect the cache and upload to my server to improve the initialization speed 
                            string url = string.Format( "http://jerrywang.zoka.cc/cache/upload.php?hash={0}", Path.GetFileNameWithoutExtension(file));
                            HttpWebRequest request = HttpWebRequest.Create(url) as HttpWebRequest;
                            request.Method = "POST";

                            byte[] buffer = null;
                            using (FileStream fs = new FileStream(file, FileMode.Open, FileAccess.Read) )
                            {
                                buffer = new byte[fs.Length];
                                fs.Read(buffer, 0, buffer.Length);
                                fs.Close();
                            }

                            request.ContentLength = buffer.Length;
                            Stream requestStream = request.GetRequestStream();
                            requestStream.Write(buffer, 0, buffer.Length);
                            requestStream.Close();

                            using (Stream responseStream = request.GetResponse().GetResponseStream())
                            {
                                using (StreamReader sr = new StreamReader(responseStream))
                                {
                                    string text = sr.ReadToEnd();
                                    Trace.WriteLine(text);
                                }
                            }
                        }
                        catch (System.Exception ex)
                        {
                    	
                        }
                    }// foreach
                }// if
            }
        }

        private delegate void InitializationCompletedDelegate(InjectionHelper.Status status);
        private void InitializationCompleted(InjectionHelper.Status status)
        {
            if( status == InjectionHelper.Status.Ready )
            {
                txtOutput.Text = @"Initialization is completed successfully, enjoy!";
                foreach (Control ctrl in this.Controls)
                {
                    if (ctrl is Button)
                        ctrl.Enabled = true;
                }
                txtOutput.Enabled = true;
            }
            else
            {
                txtOutput.Text = string.Format( @"Initialization is failed with error [{0}]!", status.ToString());
            }
        }


        //-------------------------------------------------------
        protected string CompareOneAndTwo()
        {
            int a = 1;
            int b = 2;
            if (a < b)
            {
                return "Number 1 is less than 2";
            }
            else
            {
                return "Number 1 is greater than 2 (O_o)";
            }
        }

        protected string GenericMethodToBeReplaced<T, K>(T t, K k)
        {
            int a = 1;
            int b = 2;
            if (a > b)
                a = b;
            else
                b = a;

            return string.Format("Original generic method is being called!"
                , typeof(T).FullName
                , typeof(K).FullName
                );
        }

        protected string GenericMethodSourceILCodeToBeCopiedFrom<T, K>(T t, K k)
        {
            return string.Format("Modifed generic method is being called! Type 1 = {0}; Type 2 = {1}"
                , typeof(T).FullName
                , typeof(K).FullName
                );
        }

        private static int DynamicMethodSourceILCode()
        {
            return 2012;
        }

        private static int StaticMethod_AddTwoNumber(int a, int b)
        {
            // if a method is too short, JIT-complier complies it as inline, then we can't update it.
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            Trace.WriteLine("XXXXXXXXXXXXXXX");
            return a + b;
        }

        private static int StaticMethodSourceILCode(int a, int b)
        {
            return a - b;
        }
        //--------------------------------------------------------

        private void btnReplaceAndCall_Click(object sender, EventArgs e)
        {
            // get the target method first
            Type type = this.GetType();
            MethodInfo methodInfo = type.GetMethod("CompareOneAndTwo", BindingFlags.NonPublic | BindingFlags.Instance);

            // the following line is unnecessary actually
            // Here we use it to cause the method to be compiled by JIT
            // so that we can verify this also works for JIT-compiled method :)
            RuntimeHelpers.PrepareMethod(methodInfo.MethodHandle);

            // get the original IL Codes for the method 
            byte [] ilCodes = methodInfo.GetMethodBody().GetILAsByteArray();

            // this is not a good way to search OpCode without parsing
            // but it works for our sample :)
            for (int i = 0; i < ilCodes.Length; i++)
            {
                if (ilCodes[i] == OpCodes.Bge_S.Value)
                {
                    // Replacing Bge_S with Blt_S
                    ilCodes[i] = (byte)OpCodes.Blt_S.Value;
                }
            }

            // update the IL
            InjectionHelper.UpdateILCodes(methodInfo, ilCodes);

            // Call the method

            txtOutput.Text = string.Format(@"Update the IL for jit-compiled method and then call CompareOneAndTwo() method.

The result is ""{0}"""
                , CompareOneAndTwo()
                );
        }

        private void btnCallDirectly_Click(object sender, EventArgs e)
        {
            txtOutput.Text = string.Format(@"Call CompareOneAndTwo() method.

The result is ""{0}"""
                , CompareOneAndTwo()
                );
        }

        private delegate int DynamicMethodDelegate();
        private void btnReplaceDynamicMethod_Click(object sender, EventArgs e)
        {
            DynamicMethod dynamicMethod = new DynamicMethod("DM", typeof(int), new Type[0], this.GetType().Module);
            ILGenerator il = dynamicMethod.GetILGenerator();
            il.Emit(OpCodes.Ldc_I4, 999);
            il.Emit(OpCodes.Ret);

            DynamicMethodDelegate action = (DynamicMethodDelegate)dynamicMethod.CreateDelegate(typeof(DynamicMethodDelegate));

            txtOutput.Text = string.Format(@"First emit a dynamic method which just returns number 999, and we call it first before modifying the IL codes.
And result is [{0}].
----------------------------------

"               
                , action()
                );

            // create another DynamicMethod and get the ILCodes
            {
                Type type = this.GetType();
                MethodInfo methodInfo = type.GetMethod("DynamicMethodSourceILCode", BindingFlags.NonPublic | BindingFlags.Static);
                byte[] ilCodes = methodInfo.GetMethodBody().GetILAsByteArray();

                // update the ILCodes to the target method
                InjectionHelper.UpdateILCodes(dynamicMethod, ilCodes);
            }


            txtOutput.Text += string.Format(@"Now we updated the dynamic method IL codes with the source from DynamicMethodSourceILCode(), which is another simple method returns number 2012.

Next, we call the dynamic method again and see the result is [{0}].

---------------------
P.S. Dynamic method is very special that they don't have a MethodTable related, copying the IL code from a static-compiled method usually can cause CLR crash, so here just update the IL code with simple method. For complicated method, you'd better get the IL source from another dynamic method.
"
                , action()
                );
        }

        private void btnUpdateStaticMethod_Click(object sender, EventArgs e)
        {
            // get the target method first
            Type type = this.GetType();
            MethodInfo targetMethod = type.GetMethod("StaticMethod_AddTwoNumber", BindingFlags.NonPublic | BindingFlags.Static);
            MethodInfo srcMethod = type.GetMethod("StaticMethodSourceILCode", BindingFlags.NonPublic | BindingFlags.Static);

            txtOutput.Text = string.Format( @"Nothing new for static method, call StaticMethod_AddTwoNumber(2,1), and the result is [{0}]
------------------------------------

"
                , StaticMethod_AddTwoNumber( 2, 1)
                );

            // get the  IL Codes of StaticMethodSourceILCode
            byte[] ilCodes = srcMethod.GetMethodBody().GetILAsByteArray();

            // replace the IL
            InjectionHelper.UpdateILCodes(targetMethod, ilCodes);


            txtOutput.Text += string.Format(@"After updating the IL codes, call StaticMethod_AddTwoNumber(2,1) again, and the result is [{0}]
------------------------------------

"
                , StaticMethod_AddTwoNumber(2, 1)
                );
        }

        private void btnReplaceGeneric_Click(object sender, EventArgs e)
        {
            Type type = this.GetType();
            MethodInfo destMethodInfo = type.GetMethod("GenericMethodToBeReplaced", BindingFlags.NonPublic | BindingFlags.Instance);
            MethodInfo srcMethodInfo = type.GetMethod("GenericMethodSourceILCodeToBeCopiedFrom", BindingFlags.NonPublic | BindingFlags.Instance);

            byte[] ilCodes = srcMethodInfo.GetMethodBody().GetILAsByteArray();

            txtOutput.Text = string.Format(@"Generic methods are most complicated, see the article for details.

{0}
-----------------------------

"
                , GenericMethodToBeReplaced<string, int>("11", 2)
                );


            InjectionHelper.UpdateILCodes(destMethodInfo, ilCodes);

            txtOutput.Text += string.Format(@"After updating the IL Code which is copied from another generic method.

{0}

{1}
-----------------------------
"
                , GenericMethodToBeReplaced<string, int>("11", 2)
                , GenericMethodToBeReplaced<long, int>(1, 2)
                );


        }

        


        


    }
}
