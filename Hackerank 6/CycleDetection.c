bool has_cycle(SinglyLinkedListNode* head) {
    if(head == NULL) {
        return false;
    }
    SinglyLinkedListNode* slow = head;
    SinglyLinkedListNode* fast = head->next;
    if(fast == NULL || fast->next == NULL) {
        return false;
    }
    while(fast != NULL && fast->next != NULL) {
        if(fast == slow) {
            return true;
        }
        fast = fast->next->next;
        slow = slow->next;
    }
    return false;
    

}
