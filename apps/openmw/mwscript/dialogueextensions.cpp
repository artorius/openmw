
#include "dialogueextensions.hpp"

#include <components/compiler/extensions.hpp>

#include <components/interpreter/interpreter.hpp>
#include <components/interpreter/runtime.hpp>
#include <components/interpreter/opcodes.hpp>

#include "../mwdialogue/journal.hpp"
#include "../mwdialogue/dialoguemanager.hpp"

#include "interpretercontext.hpp"

namespace MWScript
{
    namespace Dialogue
    {
        class OpJournal : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWScript::InterpreterContext& context
                        = static_cast<MWScript::InterpreterContext&> (runtime.getContext());

                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    Interpreter::Type_Integer index = runtime[0].mInteger;
                    runtime.pop();

                    context.getEnvironment().mJournal->addEntry (quest, index);
                }
        };

        class OpSetJournalIndex : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWScript::InterpreterContext& context
                        = static_cast<MWScript::InterpreterContext&> (runtime.getContext());

                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    Interpreter::Type_Integer index = runtime[0].mInteger;
                    runtime.pop();

                    context.getEnvironment().mJournal->setJournalIndex (quest, index);
                }
        };

        class OpGetJournalIndex : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWScript::InterpreterContext& context
                        = static_cast<MWScript::InterpreterContext&> (runtime.getContext());

                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    int index = context.getEnvironment().mJournal->getJournalIndex (quest);

                    runtime.push (index);

                }
        };

        class OpAddTopic : public Interpreter::Opcode0
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime)
                {
                    MWScript::InterpreterContext& context
                        = static_cast<MWScript::InterpreterContext&> (runtime.getContext());

                    std::string topic = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    context.getEnvironment().mDialogueManager->addTopic(topic);
                }
        };

        class OpChoice : public Interpreter::Opcode1
        {
            public:

                virtual void execute (Interpreter::Runtime& runtime, unsigned int arg0)
                {
                    MWScript::InterpreterContext& context
                        = static_cast<MWScript::InterpreterContext&> (runtime.getContext());
                    MWDialogue::DialogueManager* dialogue = context.getEnvironment().mDialogueManager;
                    while(arg0>0)
                    {
                        std::string question = runtime.getStringLiteral (runtime[0].mInteger);
                        runtime.pop();
                        arg0 = arg0 -1;
                        Interpreter::Type_Integer choice = 1;
                        if(arg0>0)
                        {
                            choice = runtime[0].mInteger;
                            runtime.pop();
                            arg0 = arg0 -1;
                        }
                        dialogue->askQuestion(question,choice);
                    }
                }
        };


        const int opcodeJournal = 0x2000133;
        const int opcodeSetJournalIndex = 0x2000134;
        const int opcodeGetJournalIndex = 0x2000135;
        const int opcodeAddTopic = 0x200013a;
        const int opcodeChoice = 0x2000a;

        void registerExtensions (Compiler::Extensions& extensions)
        {
            extensions.registerInstruction ("journal", "cl", opcodeJournal);
            extensions.registerInstruction ("setjournalindex", "cl", opcodeSetJournalIndex);
            extensions.registerFunction ("getjournalindex", 'l', "c", opcodeGetJournalIndex);
            extensions.registerInstruction ("addtopic", "S" , opcodeAddTopic);
            extensions.registerInstruction ("choice", "/SlSlSlSlSlSlSlSlSlSlSlSlSlSlSlSl", opcodeChoice);
        }

        void installOpcodes (Interpreter::Interpreter& interpreter)
        {
            interpreter.installSegment5 (opcodeJournal, new OpJournal);
            interpreter.installSegment5 (opcodeSetJournalIndex, new OpSetJournalIndex);
            interpreter.installSegment5 (opcodeGetJournalIndex, new OpGetJournalIndex);
            interpreter.installSegment5 (opcodeAddTopic, new OpAddTopic);
            interpreter.installSegment3 (opcodeChoice,new OpChoice);
        }
    }

}
