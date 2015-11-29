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
using Microsoft.Win32;

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
This may take a short while if the addresses can't be found in cache.";
            InjectionHelper.Initialize();

            Thread thread = new Thread(WaitForInitialization);
            thread.Start();
        }

        private void WaitForInitialization()
        {
            bool success = false;
            InitializationCompletedDelegate del = new InitializationCompletedDelegate(InitializationCompleted);
            try
            {
                InjectionHelper.WaitForIntializationCompletion();
                this.BeginInvoke(del, true, null);

                success = true;
            }
            catch (Exception ex)
            {
                this.BeginInvoke(del, false, ex.Message);
            }

            if (success)
            {
                try
                {
                    CollentAllVersions();       
                }
                catch
                {
                }                         
            }            
        }


        private delegate void InitializationCompletedDelegate(bool isSuccess, string errorMessage);
        private void InitializationCompleted(bool isSuccess, string errorMessage)
        {
            if (isSuccess)
            {
                txtOutput.Text = @"Initialization is completed successfully!";
                foreach (Control ctrl in this.Controls)
                {
                    if (ctrl is Button)
                        ctrl.Enabled = true;
                }
                txtOutput.Enabled = true;
            }
            else
            {
                txtOutput.Text = string.Format(@"Initialization is failed with error [{0}]!", errorMessage);
            }
        }


        //-------------------------------------------------------
        #region hook
        private string TargetMethod(string a, string b)
        {
            Trace.WriteLine("Target method is called.");
            return a + "," + b;
        }

        private string ReplaceMethod(string a, string b)
        {
            return string.Format( "This method is hooked, a={0};b={1};", a, b);
        }

        private void btnCallTargetMethod_Click(object sender, EventArgs e)
        {
            txtOutput.Text = string.Format(@"
CALL TargetMethod(""Hello"", ""World""); 
Result : 

{0}"
                , TargetMethod("Hello", "World")
                );
        }

        private void btnHook_Click(object sender, EventArgs e)
        {
            Type type = this.GetType();
            MethodInfo targetMethod = type.GetMethod("TargetMethod", BindingFlags.NonPublic | BindingFlags.Instance);
            MethodInfo replaceMethod = type.GetMethod("ReplaceMethod", BindingFlags.NonPublic | BindingFlags.Instance);

            byte[] ilCodes = new byte[5];
            ilCodes[0] = (byte)OpCodes.Jmp.Value;
            ilCodes[1] = (byte)(replaceMethod.MetadataToken & 0xFF);
            ilCodes[2] = (byte)(replaceMethod.MetadataToken >> 8 & 0xFF);
            ilCodes[3] = (byte)(replaceMethod.MetadataToken >> 16 & 0xFF);
            ilCodes[4] = (byte)(replaceMethod.MetadataToken >> 24 & 0xFF);

            InjectionHelper.UpdateILCodes(targetMethod, ilCodes);

            txtOutput.Text = "TargetMethod is hooked";
        }
        #endregion



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

        


        ////////////////////////////////////////
        #region the following code collects all the JIT + CLR dll versions on this machine to help improve the code
        private static void CollentAllVersions()
        {
            // This is the location of the .Net Framework Registry Key
            string framworkRegPath = @"Software\Microsoft\.NetFramework";

            // Get a non-writable key from the registry
            RegistryKey netFramework = Registry.LocalMachine.OpenSubKey(framworkRegPath, false);

            // Retrieve the install root path for the framework
            string installRoot = netFramework.GetValue("InstallRoot").ToString();
            if (Directory.Exists(installRoot))
            {
                string[] dirs = Directory.GetDirectories(installRoot, "*", SearchOption.TopDirectoryOnly);
                foreach (string dir in dirs)
                {
                    string path = Path.Combine(dir, "clrjit.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "mscorjit.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "clr.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "mscorwks.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);
                }
            }
            

            // search in another directory
            installRoot = installRoot.TrimEnd('\\', '/');
            if (installRoot.EndsWith("Framework", StringComparison.InvariantCultureIgnoreCase))
                installRoot = Path.Combine(Path.GetDirectoryName(installRoot), "Framework64");
            else
                installRoot = Path.Combine(Path.GetDirectoryName(installRoot), "Framework");

            if (Directory.Exists(installRoot))
            {
                string [] dirs = Directory.GetDirectories(installRoot, "*", SearchOption.TopDirectoryOnly);
                foreach (string dir in dirs)
                {
                    string path = Path.Combine(dir, "clrjit.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "mscorjit.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "clr.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);

                    path = Path.Combine(dir, "mscorwks.dll");
                    if (File.Exists(path))
                        InjectionHelper.CollectVersion(path);
                }
            }
        }
        #endregion

        

    }
}
