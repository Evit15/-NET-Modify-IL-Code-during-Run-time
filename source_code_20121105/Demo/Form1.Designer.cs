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
            this.SuspendLayout();
            // 
            // btnReplaceAndCall
            // 
            this.btnReplaceAndCall.Location = new System.Drawing.Point(23, 54);
            this.btnReplaceAndCall.Name = "btnReplaceAndCall";
            this.btnReplaceAndCall.Size = new System.Drawing.Size(234, 24);
            this.btnReplaceAndCall.TabIndex = 0;
            this.btnReplaceAndCall.Text = "Modify the IL and call \'CompareOneAndTwo\'";
            this.btnReplaceAndCall.UseVisualStyleBackColor = true;
            this.btnReplaceAndCall.Click += new System.EventHandler(this.btnReplaceAndCall_Click);
            // 
            // btnCallDirectly
            // 
            this.btnCallDirectly.Location = new System.Drawing.Point(23, 23);
            this.btnCallDirectly.Name = "btnCallDirectly";
            this.btnCallDirectly.Size = new System.Drawing.Size(234, 23);
            this.btnCallDirectly.TabIndex = 1;
            this.btnCallDirectly.Text = "Call \'CompareOneAndTwo\' method directly";
            this.btnCallDirectly.UseVisualStyleBackColor = true;
            this.btnCallDirectly.Click += new System.EventHandler(this.btnCallDirectly_Click);
            // 
            // btnReplaceGeneric
            // 
            this.btnReplaceGeneric.Location = new System.Drawing.Point(23, 107);
            this.btnReplaceGeneric.Name = "btnReplaceGeneric";
            this.btnReplaceGeneric.Size = new System.Drawing.Size(234, 23);
            this.btnReplaceGeneric.TabIndex = 2;
            this.btnReplaceGeneric.Text = "Modify Generic Method IL Code";
            this.btnReplaceGeneric.UseVisualStyleBackColor = true;
            this.btnReplaceGeneric.Click += new System.EventHandler(this.btnReplaceGeneric_Click);
            // 
            // btnReplaceDynamicMethod
            // 
            this.btnReplaceDynamicMethod.Location = new System.Drawing.Point(23, 136);
            this.btnReplaceDynamicMethod.Name = "btnReplaceDynamicMethod";
            this.btnReplaceDynamicMethod.Size = new System.Drawing.Size(234, 23);
            this.btnReplaceDynamicMethod.TabIndex = 3;
            this.btnReplaceDynamicMethod.Text = "Modify Dynamic Method IL Code";
            this.btnReplaceDynamicMethod.UseVisualStyleBackColor = true;
            this.btnReplaceDynamicMethod.Click += new System.EventHandler(this.btnReplaceDynamicMethod_Click);
            // 
            // btnUpdateStaticMethod
            // 
            this.btnUpdateStaticMethod.Location = new System.Drawing.Point(23, 167);
            this.btnUpdateStaticMethod.Name = "btnUpdateStaticMethod";
            this.btnUpdateStaticMethod.Size = new System.Drawing.Size(234, 23);
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
            this.txtOutput.Location = new System.Drawing.Point(324, 0);
            this.txtOutput.Multiline = true;
            this.txtOutput.Name = "txtOutput";
            this.txtOutput.ReadOnly = true;
            this.txtOutput.ShortcutsEnabled = false;
            this.txtOutput.Size = new System.Drawing.Size(499, 366);
            this.txtOutput.TabIndex = 6;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(823, 366);
            this.Controls.Add(this.txtOutput);
            this.Controls.Add(this.btnUpdateStaticMethod);
            this.Controls.Add(this.btnReplaceDynamicMethod);
            this.Controls.Add(this.btnReplaceGeneric);
            this.Controls.Add(this.btnCallDirectly);
            this.Controls.Add(this.btnReplaceAndCall);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
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
    }
}

