/*
 * Copyright (C) 2005-2019 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if SWIGPYTHON

%module(directors="1") cb
%{
   #include "otbPythonLogOutput.h"
   #include "otbLogOutputCallback.h"
%}

%feature("director") LogOutputCallback;

%include "otbLogOutputCallback.h"

class itkLogOutput : public itkObject
{
protected:
  itkLogOutput();
  ~itkLogOutput();
};

%pythoncode
  {
  class PythonLogOutputCallback(LogOutputCallback):
      def __init__(self):
          super(PythonLogOutputCallback, self).__init__()
      def Call(self, content):
          print(content)
      def Flush(self):
          sys.stdout.flush()
  }

class PythonLogOutput : public itkLogOutput
{
public:
  static PythonLogOutput_Pointer New();
  virtual void Delete();
  void SetCallback(otb::LogOutputCallback* callback);
  virtual void Write(std::string const & content);

protected:
  PythonLogOutput();
  //~PythonLogOutput();
};

DECLARE_REF_COUNT_CLASS( PythonLogOutput )

class Logger
{
public:
  virtual void AddLogOutput(itkLogOutput *output);
  static Logger * Instance();
  void ResetOutputs();
protected:
  Logger();
  virtual ~Logger();

};

%pythoncode {
  libraryLogOutput = PythonLogOutput_New()
  libraryLogCallback = PythonLogOutputCallback()
  Logger.Instance().ResetOutputs()
  libraryLogOutput.SetCallback(libraryLogCallback)
  Logger.Instance().AddLogOutput(libraryLogOutput.GetPointer())
}

#endif
