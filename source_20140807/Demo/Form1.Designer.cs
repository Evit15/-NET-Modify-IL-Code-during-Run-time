namespace HelloWorld_V4
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnReplaceAndCall = new System.Windows.Forms.Button();
            this.btnCallDirectly = new System.Windows.Forms.Button();
            this.btnReplaceGeneric = new System.Windows.Forms.Button();
            this.btnReplaceDynamicMethod = new System.Windows.Forms.Button();
            this.btnUpdateStaticMethod = new System.Windows.Forms.Button();
            this.txtOutput = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.btnHook = new System.Windows.Forms.Button();
            this.btnCallTargetMethod = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnReplaceAndCall
            // 
            this.btnReplaceAndCall.Location = new System.Drawing.Point(15, 57);
            this.btnReplaceAndCall.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnReplaceAndCall.Name = "btnReplaceAndCall";
            this.btnReplaceAndCall.Size = new System.Drawing.Size(312, 30);
            this.btnReplaceAndCall.TabIndex = 0;
            this.btnReplaceAndCall.Text = "Modify the IL and call \'CompareOneAndTwo\'";
            this.btnReplaceAndCall.UseVisualStyleBackColor = true;
            this.btnReplaceAndCall.Click += new System.EventHandler(this.btnReplaceAndCall_Click);
            // 
            // btnCallDirectly
            // 
            this.btnCallDirectly.Location = new System.Drawing.Point(15, 19);
            this.btnCallDirectly.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnCallDirectly.Name = "btnCallDirectly";
            this.btnCallDirectly.Size = new System.Drawing.Size(312, 28);
            this.btnCallDirectly.TabIndex = 1;
            this.btnCallDirectly.Text = "Call \'CompareOneAndTwo\' method directly";
            this.btnCallDirectly.UseVisualStyleBackColor = true;
            this.btnCallDirectly.Click += new System.EventHandler(this.btnCallDirectly_Click);
            // 
            // btnReplaceGeneric
            // 
            this.btnReplaceGeneric.Location = new System.Drawing.Point(31, 324);
            this.btnReplaceGeneric.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnReplaceGeneric.Name = "btnReplaceGeneric";
            this.btnReplaceGeneric.Size = new System.Drawing.Size(312, 28);
            this.btnReplaceGeneric.TabIndex = 2;
            this.btnReplaceGeneric.Text = "Modify Generic Method IL Code";
            this.btnReplaceGeneric.UseVisualStyleBackColor = true;
            this.btnReplaceGeneric.Click += new System.EventHandler(this.btnReplaceGeneric_Click);
            // 
            // btnReplaceDynamicMethod
            // 
            this.btnReplaceDynamicMethod.Location = new System.Drawing.Point(31, 359);
            this.btnReplaceDynamicMethod.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnReplaceDynamicMethod.Name = "btnReplaceDynamicMethod";
            this.btnReplaceDynamicMethod.Size = new System.Drawing.Size(312, 28);
            this.btnReplaceDynamicMethod.TabIndex = 3;
            this.btnReplaceDynamicMethod.Text = "Modify Dynamic Method IL Code";
            this.btnReplaceDynamicMethod.UseVisualStyleBackColor = true;
            this.btnReplaceDynamicMethod.Click += new System.EventHandler(this.btnReplaceDynamicMethod_Click);
            // 
            // btnUpdateStaticMethod
            // 
            this.btnUpdateStaticMethod.Location = new System.Drawing.Point(31, 398);
            this.btnUpdateStaticMethod.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnUpdateStaticMethod.Name = "btnUpdateStaticMethod";
            this.btnUpdateStaticMethod.Size = new System.Drawing.Size(312, 28);
            this.btnUpdateStaticMethod.TabIndex = 4;
            this.btnUpdateStaticMethod.Text = "Modify Static Method IL Code";
            this.btnUpdateStaticMethod.UseVisualStyleBackColor = true;
            this.btnUpdateStaticMethod.Click += new System.EventHandler(this.btnUpdateStaticMethod_Click);
            // 
            // txtOutput
            // 
            this.txtOutput.Dock = System.Windows.Forms.DockStyle.Right;
            this.txtOutput.Enabled = false;
            this.txtOutput.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtOutput.Location = new System.Drawing.Point(433, 0);
            this.txtOutput.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.txtOutput.Multiline = true;
            this.txtOutput.Name = "txtOutput";
            this.txtOutput.ReadOnly = true;
            this.txtOutput.ShortcutsEnabled = false;
            this.txtOutput.Size = new System.Drawing.Size(664, 450);
            this.txtOutput.TabIndex = 6;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnReplaceAndCall);
            this.groupBox1.Controls.Add(this.btnCallDirectly);
            this.groupBox1.Location = new System.Drawing.Point(21, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(338, 102);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.btnCallTargetMethod);
            this.groupBox2.Controls.Add(this.btnHook);
            this.groupBox2.Location = new System.Drawing.Point(21, 140);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(338, 107);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            // 
            // btnHook
            // 
            this.btnHook.Location = new System.Drawing.Point(16, 61);
            this.btnHook.Margin = new System.Windows.Forms.Padding(4);
            this.btnHook.Name = "btnHook";
            this.btnHook.Size = new System.Drawing.Size(312, 30);
            this.btnHook.TabIndex = 2;
            this.btnHook.Text = "Hook Target Method";
            this.btnHook.UseVisualStyleBackColor = true;
            this.btnHook.Click += new System.EventHandler(this.btnHook_Click);
            // 
            // btnCallTargetMethod
            // 
            this.btnCallTargetMethod.Location = new System.Drawing.Point(16, 23);
            this.btnCallTargetMethod.Margin = new System.Windows.Forms.Padding(4);
            this.btnCallTargetMethod.Name = "btnCallTargetMethod";
            this.btnCallTargetMethod.Size = new System.Drawing.Size(312, 30);
            this.btnCallTargetMethod.TabIndex = 3;
            this.btnCallTargetMethod.Text = "Call Target Method";
            this.btnCallTargetMethod.UseVisualStyleBackColor = true;
            this.btnCallTargetMethod.Click += new System.EventHandler(this.btnCallTargetMethod_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1097, 450);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.txtOutput);
            this.Controls.Add(this.btnUpdateStaticMethod);
            this.Controls.Add(this.btnReplaceDynamicMethod);
            this.Controls.Add(this.btnReplaceGeneric);
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnReplaceAndCall;
        private System.Windows.Forms.Button btnCallDirectly;
        private System.Windows.Forms.Button btnReplaceGeneric;
        private System.Windows.Forms.Button btnReplaceDynamicMethod;
        private System.Windows.Forms.Button btnUpdateStaticMethod;
        private System.Windows.Forms.TextBox txtOutput;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button btnCallTargetMethod;
        private System.Windows.Forms.Button btnHook;
    }
}

