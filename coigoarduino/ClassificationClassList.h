#ifndef __CLASSIFICATIONCLASSLIST_H__
#define __CLASSIFICATIONCLASSLIST_H__

struct ClassificationDetectionItem {
    uint8_t index;
    const char* imgclassName;
    uint8_t filter;
};

// List of objects the pre-trained model is capable of recognizing
// Index number is fixed and hard-coded from training
// Set the filter value to 0 to ignore any recognized objects
ClassificationDetectionItem imgclassItemList[6] = {
    {0, "Sarna", 1},
    {1, "Podredumbre",     1},
    {2, "Hoja_Sana",     1},
    {3, "aranita",     1},
    {4, "escarabjo",   1}
};

#endif
