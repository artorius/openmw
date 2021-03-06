#include "messagebox.hpp"

using namespace MWGui;

MessageBoxManager::MessageBoxManager (WindowManager *windowManager)
{
    mWindowManager = windowManager;
    // defines
    mMessageBoxSpeed = 0.1;
    mInterMessageBoxe = NULL;
}

void MessageBoxManager::onFrame (float frameDuration)
{
    std::vector<MessageBoxManagerTimer>::iterator it;
    for(it = mTimers.begin(); it != mTimers.end();)
    {
        it->current += frameDuration;
        if(it->current >= it->max)
        {
            it->messageBox->mMarkedToDelete = true;

            if(*mMessageBoxes.begin() == it->messageBox) // if this box is the last one
            {
                // collect all with mMarkedToDelete and delete them.
                // and place the other messageboxes on the right position
                int height = 0;
                std::vector<MessageBox*>::iterator it2 = mMessageBoxes.begin();
                while(it2 != mMessageBoxes.end())
                {
                    if((*it2)->mMarkedToDelete)
                    {
                        delete (*it2);
                        it2 = mMessageBoxes.erase(it2);
                    }
                    else {
                        (*it2)->update(height);
                        height += (*it2)->getHeight();
                        it2++;
                    }
                }
            }
            it = mTimers.erase(it);
        }
        else
        {
            it++;
        }
    }

    if(mInterMessageBoxe != NULL && mInterMessageBoxe->mMarkedToDelete) {
        delete mInterMessageBoxe;
        mInterMessageBoxe = NULL;
        mWindowManager->setNextMode(GM_Game);
    }
}

void MessageBoxManager::createMessageBox (const std::string& message)
{
    MessageBox *box = new MessageBox(*this, message);

    removeMessageBox(message.length()*mMessageBoxSpeed, box);

    mMessageBoxes.push_back(box);
    std::vector<MessageBox*>::iterator it;

    if(mMessageBoxes.size() > 3) {
        delete *mMessageBoxes.begin();
        mMessageBoxes.erase(mMessageBoxes.begin());
    }

    int height = 0;
    for(it = mMessageBoxes.begin(); it != mMessageBoxes.end(); ++it)
    {
        (*it)->update(height);
        height += (*it)->getHeight();
    }
}

bool MessageBoxManager::createInteractiveMessageBox (const std::string& message, const std::vector<std::string>& buttons)
{
    if(mInterMessageBoxe != NULL) {
        std::cout << "there is a MessageBox already" << std::endl;
        return false;
    }
    std::cout << "interactive MessageBox: " << message << " - ";
    std::copy (buttons.begin(), buttons.end(), std::ostream_iterator<std::string> (std::cout, ", "));
    std::cout << std::endl;

    mInterMessageBoxe = new InteractiveMessageBox(*this, message, buttons);

    return true;
}

bool MessageBoxManager::isInteractiveMessageBox ()
{
    return mInterMessageBoxe != NULL;
}

void MessageBoxManager::removeMessageBox (float time, MessageBox *msgbox)
{
    MessageBoxManagerTimer timer;
    timer.current = 0;
    timer.max = time;
    timer.messageBox = msgbox;

    mTimers.insert(mTimers.end(), timer);
}

bool MessageBoxManager::removeMessageBox (MessageBox *msgbox)
{
    std::vector<MessageBox*>::iterator it;
    for(it = mMessageBoxes.begin(); it != mMessageBoxes.end(); ++it)
    {
        if((*it) == msgbox)
        {
            delete (*it);
            mMessageBoxes.erase(it);
            return true;
        }
    }
    return false;
}

void MessageBoxManager::setMessageBoxSpeed (int speed)
{
    mMessageBoxSpeed = speed;
}


int MessageBoxManager::readPressedButton ()
{
    if(mInterMessageBoxe != NULL)
    {
        return mInterMessageBoxe->readPressedButton();
    }
    return -1;
}




MessageBox::MessageBox(MessageBoxManager& parMessageBoxManager, const std::string& message)
  : Layout("openmw_messagebox_layout.xml")
  , mMessageBoxManager(parMessageBoxManager)
  , cMessage(message)
{
    // defines
    mFixedWidth = 300;
    mBottomPadding = 20;
    mNextBoxPadding = 20;
    mMarkedToDelete = false;

    getWidget(mMessageWidget, "message");

    mMessageWidget->setOverflowToTheLeft(true);
    mMessageWidget->addText(cMessage);

    MyGUI::IntSize size;
    size.width = mFixedWidth;
    size.height = 100; // dummy

    MyGUI::IntCoord coord;
    coord.left = 10; // dummy
    coord.top = 10; // dummy

    mMessageWidget->setSize(size);

    MyGUI::IntSize textSize = mMessageWidget->_getTextSize();
    size.height = mHeight = textSize.height + 20; // this is the padding between the text and the box

    mMainWidget->setSize(size);
    size.width -= 15; // this is to center the text (see messagebox_layout.xml, Widget type="Edit" position="-2 -3 0 0")
    mMessageWidget->setSize(size);
}

void MessageBox::update (int height)
{
    MyGUI::IntSize gameWindowSize = mMessageBoxManager.mWindowManager->getGui()->getViewSize();
    MyGUI::IntCoord coord;
    coord.left = (gameWindowSize.width - mFixedWidth)/2;
    coord.top = (gameWindowSize.height - mHeight - height - mBottomPadding);

    MyGUI::IntSize size;
    size.width = mFixedWidth;
    size.height = mHeight;

    mMainWidget->setCoord(coord);
    mMainWidget->setSize(size);
    mMainWidget->setVisible(true);
}

int MessageBox::getHeight ()
{
    return mHeight+mNextBoxPadding; // 20 is the padding between this and the next MessageBox
}



InteractiveMessageBox::InteractiveMessageBox(MessageBoxManager& parMessageBoxManager, const std::string& message, const std::vector<std::string>& buttons)
  : Layout("openmw_interactive_messagebox_layout.xml")
  , mMessageBoxManager(parMessageBoxManager)
  , mButtonPressed(-1)
{
    int fixedWidth = 500;
    int textPadding = 10; // padding between text-widget and main-widget
    int textButtonPadding = 20; // padding between the text-widget und the button-widget
    int buttonLeftPadding = 10; // padding between the buttons if horizontal
    int buttonTopPadding = 5; // ^-- if vertical
    int buttonPadding = 5; // padding between button label and button itself
    int buttonMainPadding = 10; // padding between buttons and bottom of the main widget

    mMarkedToDelete = false;


    getWidget(mMessageWidget, "message");
    getWidget(mButtonsWidget, "buttons");

    mMessageWidget->setOverflowToTheLeft(true);
    mMessageWidget->addText(message);

    MyGUI::IntSize textSize = mMessageWidget->_getTextSize();

    MyGUI::IntSize gameWindowSize = mMessageBoxManager.mWindowManager->getGui()->getViewSize();

    int biggestButtonWidth = 0;
    int buttonWidth = 0;
    int buttonsWidth = 0;
    int buttonHeight = 0;
    MyGUI::IntCoord dummyCoord(0, 0, 0, 0);

    std::vector<std::string>::const_iterator it;
    for(it = buttons.begin(); it != buttons.end(); ++it)
    {
        MyGUI::ButtonPtr button = mButtonsWidget->createWidget<MyGUI::Button>(
            MyGUI::WidgetStyle::Child,
            std::string("MW_Button"),
            dummyCoord,
            MyGUI::Align::Default);
        button->setCaption(*it);

        button->eventMouseButtonClick = MyGUI::newDelegate(this, &InteractiveMessageBox::mousePressed);

        mButtons.push_back(button);

        buttonWidth = button->_getTextSize().width + 2*buttonPadding + buttonLeftPadding;
        buttonsWidth += buttonWidth;
        buttonHeight = button->_getTextSize().height + 2*buttonPadding + buttonTopPadding;

        if(buttonWidth > biggestButtonWidth)
        {
            biggestButtonWidth = buttonWidth;
        }
    }
    buttonsWidth += buttonLeftPadding;

    MyGUI::IntSize mainWidgetSize;
    if(buttonsWidth < fixedWidth)
    {
        // on one line
        std::cout << "on one line" << std::endl;

        if(textSize.width + 2*textPadding < buttonsWidth)
        {
            std::cout << "width = buttonsWidth" << std::endl;
            mainWidgetSize.width = buttonsWidth;
        }
        else
        {
            mainWidgetSize.width = textSize.width + 3*textPadding;
        }
        mainWidgetSize.height = textSize.height + textButtonPadding + buttonHeight + buttonMainPadding;

        MyGUI::IntCoord absCoord;
        absCoord.left = (gameWindowSize.width - mainWidgetSize.width)/2;
        absCoord.top = (gameWindowSize.height - mainWidgetSize.height)/2;

        std::cout << "width " << mainWidgetSize.width << " height " << mainWidgetSize.height << std::endl;
        std::cout << "left " << absCoord.left << " top " << absCoord.top << std::endl;

        mMainWidget->setCoord(absCoord);
        mMainWidget->setSize(mainWidgetSize);


        MyGUI::IntCoord messageWidgetCoord;
        messageWidgetCoord.left = (mainWidgetSize.width - textSize.width)/2;
        messageWidgetCoord.top = textPadding;
        mMessageWidget->setCoord(messageWidgetCoord);

        mMessageWidget->setSize(textSize);

        MyGUI::IntCoord buttonCord;
        MyGUI::IntSize buttonSize(0, buttonHeight);
        int left = (mainWidgetSize.width - buttonsWidth)/2 + buttonPadding;

        std::vector<MyGUI::ButtonPtr>::const_iterator button;
        for(button = mButtons.begin(); button != mButtons.end(); ++button)
        {
            buttonCord.left = left;
            buttonCord.top = textSize.height + textButtonPadding;

            buttonSize.width = (*button)->_getTextSize().width + 2*buttonPadding;
            buttonSize.height = (*button)->_getTextSize().height + 2*buttonPadding;

            (*button)->setCoord(buttonCord);
            (*button)->setSize(buttonSize);

            left += buttonSize.width + buttonLeftPadding;
        }
    }
    else
    {
        // among each other

        if(biggestButtonWidth > textSize.width) {
            mainWidgetSize.width = biggestButtonWidth + buttonTopPadding;
        }
        else {
            mainWidgetSize.width = textSize.width + 3*textPadding;
        }
        mainWidgetSize.height = textSize.height + 2*textPadding + textButtonPadding + buttonHeight * buttons.size() + buttonMainPadding;

        std::cout << "biggestButtonWidth " << biggestButtonWidth << " textSize.width " << textSize.width << std::endl;
        std::cout << "width " << mainWidgetSize.width << " height " << mainWidgetSize.height << std::endl;
        mMainWidget->setSize(mainWidgetSize);

        MyGUI::IntCoord absCoord;
        absCoord.left = (gameWindowSize.width - mainWidgetSize.width)/2;
        absCoord.top = (gameWindowSize.height - mainWidgetSize.height)/2;

        mMainWidget->setCoord(absCoord);
        mMainWidget->setSize(mainWidgetSize);


        MyGUI::IntCoord messageWidgetCoord;
        messageWidgetCoord.left = (mainWidgetSize.width - textSize.width)/2;
        messageWidgetCoord.top = textPadding;
        mMessageWidget->setCoord(messageWidgetCoord);

        mMessageWidget->setSize(textSize);

        MyGUI::IntCoord buttonCord;
        MyGUI::IntSize buttonSize(0, buttonHeight);

        int top = textButtonPadding + buttonTopPadding + textSize.height;

        std::vector<MyGUI::ButtonPtr>::const_iterator button;
        for(button = mButtons.begin(); button != mButtons.end(); ++button)
        {
            buttonSize.width = (*button)->_getTextSize().width + buttonPadding*2;
            buttonSize.height = (*button)->_getTextSize().height + buttonPadding*2;

            buttonCord.top = top;
            buttonCord.left = (mainWidgetSize.width - buttonSize.width)/2 - 5; // FIXME: -5 is not so nice :/

            (*button)->setCoord(buttonCord);
            (*button)->setSize(buttonSize);

            top += buttonSize.height + 2*buttonTopPadding;
        }

    }
}

void InteractiveMessageBox::mousePressed (MyGUI::Widget* pressed)
{
    mMarkedToDelete = true;
    int index = 0;
    std::vector<MyGUI::ButtonPtr>::const_iterator button;
    for(button = mButtons.begin(); button != mButtons.end(); ++button)
    {
        if(*button == pressed)
        {
            mButtonPressed = index;
            return;
        }
        index++;
    }
    std::cout << "Cant be possible :/" << std::endl;
}

int InteractiveMessageBox::readPressedButton ()
{
    int pressed = mButtonPressed;
    mButtonPressed = -1;
    return pressed;
}
