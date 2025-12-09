#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define DATAFILE "students.csv"
#define EXPORTFILE "students_export.txt"
#define BACKUP_PREFIX "students_backup_"
#define MAX_LINE 1024
#define MAX_STUDENTS 20000
#define PAGE_SIZE 10

typedef enum { ROLE_ADMIN=1, ROLE_STAFF, ROLE_GUEST } UserRole;

typedef enum { ACTIVE, INACTIVE, GRADUATED, SUSPENDED } Status;

const char *status_str[] = { "Active", "Inactive", "Graduated", "Suspended" };

typedef struct {
    char s_ID[32];
    char full_name[100];
    char dob[16];
    char gender[16];
    char contact[32];
    char email[100];
    char address[200];
    char emergency_contact[64];
    char enrollment_date[16];
    char program[100];
    int year;
    int semester;
    Status status;
    char created_date[20];
    char modified_date[20];
    char created_by[50];
    char modified_by[50];
} Student;

Student students[MAX_STUDENTS];
int count = 0;
void print_search_menu();
void add_student();
void update_student();
void delete_student();
void search_students_flow();
void list_students();
void view_student_by_id();
void export_students_txt();
void backup_datafile();
void create_sample_data();
void print_help();
void AdminMenu();
void StaffMenu();
void GuestMenu();
UserRole LogIn();
void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n == 0) return;
    if (s[n-1] == '\n') s[n-1] = '\0';
    if (n>1 && s[n-2] == '\r') s[n-2] = '\0';
}

char *strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return (char *)haystack;
    for (; *haystack; ++haystack) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            if (strncasecmp(haystack, needle, needle_len) == 0) {
                return (char *)haystack;
            }
        }
    }
    return NULL;
}
int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n-- > 0) {
        int c1 = tolower((unsigned char)*s1++);
        int c2 = tolower((unsigned char)*s2++);
        if (c1 != c2) return c1 - c2;
        if (c1 == 0) return 0;
    }
    return 0;
}

void current_datetime(char *buf, size_t buflen) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, buflen, "%Y-%m-%d %H:%M:%S", tm);
}

void current_date_simple(char *buf, size_t buflen) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, buflen, "%Y%m%d_%H%M%S", tm);
}

void write_students_to_file(Student *arr, int n) {
    FILE *f = fopen(DATAFILE, "w");
    if (!f) {
        perror("Failed to open data file for writing");
        return;
    }
    fprintf(f, "s_ID,full_name,dob,gender,contact,email,address,emergency_contact,enrollment_date,program,year,semester,status,created_date,modified_date,created_by,modified_by\n");
    for (int i=0;i<n;i++) {
        char addr_safe[200];
        strncpy(addr_safe, arr[i].address, sizeof(addr_safe));
        addr_safe[sizeof(addr_safe)-1] = '\0';
        for (char *p = addr_safe; *p; ++p) if (*p == ',') *p = ';';
        char name_safe[100];
        strncpy(name_safe, arr[i].full_name, sizeof(name_safe));
        name_safe[sizeof(name_safe)-1] = '\0';
        for (char *p = name_safe; *p; ++p) if (*p == ',') *p = ';';
        fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%s,%s,%s,%s\n",
            arr[i].s_ID, name_safe, arr[i].dob, arr[i].gender, arr[i].contact,
            arr[i].email, addr_safe, arr[i].emergency_contact, arr[i].enrollment_date,
            arr[i].program, arr[i].year, arr[i].semester, (int)arr[i].status,
            arr[i].created_date, arr[i].modified_date, arr[i].created_by, arr[i].modified_by
        );
    }
    fclose(f);
}

int read_students_from_file(Student *arr) {
    FILE *f = fopen(DATAFILE, "r");
    if (!f) {
        return 0;
    }
    char line[MAX_LINE];
    int read_count = 0;
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        Student s = {0};
        char *token;
        token = strtok(line, ",");
        if (!token) continue;
        strncpy(s.s_ID, token, sizeof(s.s_ID)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.full_name, token, sizeof(s.full_name)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.dob, token, sizeof(s.dob)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.gender, token, sizeof(s.gender)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.contact, token, sizeof(s.contact)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.email, token, sizeof(s.email)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.address, token, sizeof(s.address)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.emergency_contact, token, sizeof(s.emergency_contact)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.enrollment_date, token, sizeof(s.enrollment_date)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.program, token, sizeof(s.program)-1);

        token = strtok(NULL, ",");
        if (token) s.year = token ? atoi(token) : 0;

        token = strtok(NULL, ",");
        if (token) s.semester = token ? atoi(token) : 0;

        token = strtok(NULL, ",");
        if (token) s.status = token ? (Status)atoi(token) : ACTIVE;

        token = strtok(NULL, ",");
        if (token) strncpy(s.created_date, token, sizeof(s.created_date)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.modified_date, token, sizeof(s.modified_date)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.created_by, token, sizeof(s.created_by)-1);

        token = strtok(NULL, ",");
        if (token) strncpy(s.modified_by, token, sizeof(s.modified_by)-1);

        if (read_count < MAX_STUDENTS) arr[read_count++] = s;
    }
    fclose(f);
    return read_count;
}

int is_valid_date(const char *d) { return d && strlen(d) >= 8; }
int is_valid_email(const char *e) { return e && strchr(e, '@') && strchr(strchr(e, '@'), '.'); }
int id_exists(Student *arr, int n, const char *id) {
    for (int i=0;i<n;i++) if (strcmp(arr[i].s_ID, id) == 0) return 1;
    return 0;
}
void generate_s_ID(char *buf, size_t buflen, int index) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int year = tm->tm_year + 1900;
    snprintf(buf, buflen, "S%d-%05d", year, index);
}

void export_students_txt() {
    FILE *f = fopen(EXPORTFILE, "w");
    if (!f) { perror("Failed to open export file for writing"); return; }
    fprintf(f, "Student Records Export\nGenerated: ");
    char dt[64]; current_datetime(dt, sizeof(dt));
    fprintf(f, "%s\n\n", dt);
    for (int i=0;i<count;i++) {
        fprintf(f, "ID: %s\nName: %s\nDOB: %s\nProgram: %s\nStatus: %s\n---\n",
             students[i].s_ID, students[i].full_name, students[i].dob, students[i].program, status_str[students[i].status]);
    }
    fclose(f);
    printf("Exported %d records to %s\n", count, EXPORTFILE);
}

void backup_datafile() {
    FILE *src = fopen(DATAFILE, "r");
    if (!src) { printf("No data file to backup (%s not found).\n", DATAFILE); return; }
    char stamp[64]; current_date_simple(stamp, sizeof(stamp));
    char backupname[256];
    snprintf(backupname, sizeof(backupname), "%s%s.csv", BACKUP_PREFIX, stamp);
    FILE *dst = fopen(backupname, "w");
    if (!dst) { fclose(src); perror("Failed to create backup file"); return; }
    char buf[1024];
    while (fgets(buf, sizeof(buf), src)) fputs(buf, dst);
    fclose(src); fclose(dst);
    printf("Backup created: %s\n", backupname);
}
void create_sample_data() {
    if (count > 0) { printf("Data already exists. Sample data will not be created.\n"); return; }
    Student s1 = {0}, s2 = {0};
    generate_s_ID(s1.s_ID, sizeof(s1.s_ID), 1);
    strncpy(s1.full_name, "Alice Johnson", sizeof(s1.full_name)-1);
    strncpy(s1.program, "BSc Computer Science", sizeof(s1.program)-1);
    s1.year = 2; s1.status = ACTIVE;
    current_datetime(s1.created_date, sizeof(s1.created_date));
    strncpy(s1.created_by, "system", sizeof(s1.created_by)-1);
    strncpy(s1.modified_date, s1.created_date, sizeof(s1.modified_date)-1);

    generate_s_ID(s2.s_ID, sizeof(s2.s_ID), 2);
    strncpy(s2.full_name, "Bob Kumar", sizeof(s2.full_name)-1);
    strncpy(s2.program, "BEng Electrical", sizeof(s2.program)-1);
    s2.year = 3; s2.status = ACTIVE;
    current_datetime(s2.created_date, sizeof(s2.created_date));
    strncpy(s2.created_by, "system", sizeof(s2.created_by)-1);
    strncpy(s2.modified_date, s2.created_date, sizeof(s2.modified_date)-1);

    students[count++] = s1;
    students[count++] = s2;
    write_students_to_file(students, count);
    printf("Sample data created (2 records).\n");
}
void add_student() {
    Student s;
    memset(&s, 0, sizeof(s));
    char temp[256];
    printf("Adding new student.\n");
    generate_s_ID(s.s_ID, sizeof(s.s_ID), count+1);
    printf("Generated Student ID: %s\n", s.s_ID);

    printf("Full Name: ");
    if (!fgets(temp, sizeof(temp), stdin) || (trim_newline(temp), strlen(temp) == 0)) return;
    strncpy(s.full_name, temp, sizeof(s.full_name)-1);

    printf("Program/Course: ");
    if (!fgets(temp, sizeof(temp), stdin) || (trim_newline(temp), strlen(temp) == 0)) return;
    strncpy(s.program, temp, sizeof(s.program)-1);

    current_datetime(s.created_date, sizeof(s.created_date));
    strncpy(s.created_by, "system", sizeof(s.created_by)-1);
    strncpy(s.modified_by, "system", sizeof(s.modified_by)-1);
    strncpy(s.modified_date, s.created_date, sizeof(s.modified_date)-1);

    if (id_exists(students, count, s.s_ID)) { printf("ID exists. Cancelled.\n"); return; }

    if (count < MAX_STUDENTS) {
        students[count++] = s;
        write_students_to_file(students, count);
        printf("Student added successfully.\n");
    } else {
        printf("Storage full. Cannot add more students.\n");
    }
}

int find_student_index_by_id(const char *id) {
    for (int i=0;i<count;i++) {
        if (strcmp(students[i].s_ID, id) == 0) return i;
    }
    return -1;
}

void view_student(const Student *s) {
    printf("ID: %s | Name: %s | Program: %s | Status: %s\n", s->s_ID, s->full_name, s->program, status_str[s->status]);
    printf("DOB: %s | Contact: %s | Email: %s\n", s->dob, s->contact, s->email);
    printf("Year: %d | Semester: %d\n", s->year, s->semester);
}

void view_student_by_id() {
    char id[64];
    printf("Enter Student ID to view: ");
    if (!fgets(id, sizeof(id), stdin)) return;
    trim_newline(id);
    int idx = find_student_index_by_id(id);
    if (idx == -1) { printf("Student not found.\n"); return; }
    view_student(&students[idx]);
}

void update_student() {
    char id[64];
    char temp[256];
    printf("Enter Student ID to update: ");
    if (!fgets(id, sizeof(id), stdin)) return;
    trim_newline(id);
    int idx = find_student_index_by_id(id);
    if (idx == -1) { printf("Student not found.\n"); return; }
    Student *s = &students[idx];
    printf("Updating student '%s' (leave blank to keep current value)\n", s->full_name);

    printf("Full Name [%s]: ", s->full_name);
    if (!fgets(temp, sizeof(temp), stdin)) return;
    trim_newline(temp);
    if (strlen(temp)) strncpy(s->full_name, temp, sizeof(s->full_name)-1);

    printf("Program [%s]: ", s->program);
    if (!fgets(temp, sizeof(temp), stdin)) return;
    trim_newline(temp);
    if (strlen(temp)) strncpy(s->program, temp, sizeof(s->program)-1);

    current_datetime(s->modified_date, sizeof(s->modified_date));
    strncpy(s->modified_by, "system", sizeof(s->modified_by)-1);

    write_students_to_file(students, count);
    printf("Student updated successfully.\n");
}

void delete_student() { 
    char id[64];
    printf("Enter Student ID to delete: ");
    if (!fgets(id, sizeof(id), stdin)) return;
    trim_newline(id);
    int idx = find_student_index_by_id(id);
    if (idx == -1) { printf("Student not found.\n"); return; }
    printf("Are you sure you want to delete '%s' (Y/N)? ", students[idx].full_name);
    char c = getchar();
    clear_input();
    if (c == 'Y' || c == 'y') {
        for (int i=idx;i<count-1;i++) students[i] = students[i+1];
        count--;
        write_students_to_file(students, count);
        printf("Deleted successfully.\n");
    } else {
        printf("Deletion cancelled.\n");
    }
}
void search_by_id_brief() {
    char id[64];
    printf("Enter Student ID: ");
    if (!fgets(id, sizeof(id), stdin)) return;
    trim_newline(id);
    int idx = find_student_index_by_id(id);
    if (idx == -1) { printf("Student not found.\n"); return; }
    printf("Found: %s | %s | %s\n", students[idx].s_ID, students[idx].full_name, students[idx].program);
}

void search_by_name_brief() { 
    char q[128];
    printf("Enter Name (partial/full): ");
    if (!fgets(q, sizeof(q), stdin)) return;
    trim_newline(q);
    if (strlen(q) == 0) return;
    int found = 0;
    for (int i=0;i<count;i++) {
        if (strcasestr(students[i].full_name, q)) {
            printf("%s | %s | %s\n", students[i].s_ID, students[i].full_name, students[i].program);
            found++;
        }
    }
    if (!found) printf("No matches found.\n");
}

void search_by_program_brief() { 
    char q[128];
    printf("Enter Program name: ");
    if (!fgets(q, sizeof(q), stdin)) return;
    trim_newline(q);
    int found = 0;
    for (int i=0;i<count;i++) {
        if (strcasestr(students[i].program, q)) {
            printf("%s | %s | Year:%d\n", students[i].s_ID, students[i].full_name, students[i].year);
            found++;
        }
    }
    if (!found) printf("No matches found.\n");
}

void search_by_status_brief() {
    char temp[32];
    printf("Status (0=Active,1=Inactive,2=Graduated,3=Suspended): ");
    if (!fgets(temp, sizeof(temp), stdin)) return;
    int st = atoi(temp);
    if (st < 0 || st > 3) { printf("Invalid status.\n"); return; }
    int found = 0;
    for (int i=0;i<count;i++) {
        if ((int)students[i].status == st) {
            printf("%s | %s | %s\n", students[i].s_ID, students[i].full_name, students[i].program);
            found++;
        }
    }
    if (!found) printf("No matches found.\n");
}

void search_students_flow() {
    char opt[32];
    while (1) {
        printf("\n=== Search Menu ===\n");
        printf("1. By Student ID\n");
        printf("2. By Name (partial)\n");
        printf("3. By Program\n");
        printf("4. By Status\n");
        printf("0. Back\n");
        printf("Choose option: ");

        if (!fgets(opt, sizeof(opt), stdin)) break;
        trim_newline(opt);
        int sc = atoi(opt);
        if (sc == 1) search_by_id_brief();
        else if (sc == 2) search_by_name_brief();
        else if (sc == 3) search_by_program_brief();
        else if (sc == 4) search_by_status_brief();
        else if (sc == 0) break;
        else printf("Invalid option.\n");
    }
}

void list_students() {
    if (count == 0) { printf("No records.\n"); return; }
    printf("\n--- Student List (%d Records) ---\n", count);
    printf("ID\t\tName\t\tProgram\tStatus\n");
    printf("-----------------------------------------------------------------\n");
    for (int i=0; i < count; i++) {
        printf("%s\t%s\t%s\t%s\n",
            students[i].s_ID, students[i].full_name, students[i].program, status_str[students[i].status]);
    }
}
void print_help() {
    printf("=== Help: Student Record Management System ===\n");
    printf("Main operations:\n");
    printf(" - Add/Delete/Update: For Admin only.\n");
    printf(" - Search/Display: Available to all roles.\n");
    printf("Controls: enter the menu number and press Enter.\n");
}

void GuestMenu() {
    char opt[32];
    while (1) {
        printf("\n=== Guest Menu (Display/Search Only) ===\n");
        printf("1. Display List (List students)\n");
        printf("2. Search Students\n");
        printf("3. View student by ID\n");
        printf("4. Help / Instructions\n");
        printf("0. Log out\n");
        printf("Choose option: ");

        if (!fgets(opt, sizeof(opt), stdin)) break;
        trim_newline(opt);
        int choice = atoi(opt);
        if (choice == 1) list_students();
        else if (choice == 2) search_students_flow();
        else if (choice == 3) view_student_by_id();
        else if (choice == 4) print_help();
        else if (choice == 0) break;
        else printf("Invalid option.\n");
    }
}

void StaffMenu() {
    char opt[32];
    while (1) {
        printf("\n=== Staff Menu (Display/Search/Update) ===\n");
        printf("1. Display List (List students)\n");
        printf("2. Search Students\n");
        printf("3. View student by ID\n");
        printf("4. Update student\n");
        printf("5. Help / Instructions\n");
        printf("0. Log out\n");
        printf("Choose option: ");

        if (!fgets(opt, sizeof(opt), stdin)) break;
        trim_newline(opt);
        int choice = atoi(opt);
        if (choice == 1) list_students();
        else if (choice == 2) search_students_flow();
        else if (choice == 3) view_student_by_id();
        else if (choice == 4) update_student();
        else if (choice == 5) print_help();
        else if (choice == 0) break;
        else printf("Invalid option.\n");
    }
}

void AdminMenu() {
    char opt[32];
    while (1) {
        printf("\n=== Admin Menu (Full Control) ===\n");
        printf("1. Add new student (create)\n");
        printf("2. Delete student\n");
        printf("3. Update student\n");
        printf("4. Search students\n");
        printf("5. List students (Display)\n");
        printf("6. View student by ID\n");
        printf("7. Save and Log out\n");
        printf("8. Export students / Backup / Sample data\n");
        printf("9. Help / Instructions\n");
        printf("0. Exit without saving (Log out)\n");
        printf("Choose option: ");

        if (!fgets(opt, sizeof(opt), stdin)) break;
        trim_newline(opt);
        int choice = atoi(opt);
        if (choice == 1) add_student();
        else if (choice == 2) delete_student(); 
        else if (choice == 3) update_student(); 
        else if (choice == 4) search_students_flow(); 
        else if (choice == 5) list_students();
        else if (choice == 6) view_student_by_id();
        else if (choice == 7) {
            write_students_to_file(students, count);
            printf("Saved. Logging out.\n");
            break;
        }
        else if (choice == 8) {
            printf("--- Utilities ---\n1. Export\n2. Backup\n3. Create Sample Data\nChoose: ");
            if (!fgets(opt, sizeof(opt), stdin)) continue;
            trim_newline(opt);
            int sub_choice = atoi(opt);
            if (sub_choice == 1) export_students_txt();
            else if (sub_choice == 2) backup_datafile();
            else if (sub_choice == 3) create_sample_data();
            else printf("Invalid utility option.\n");
        }
        else if (choice == 9) print_help();
        else if (choice == 0) {
            printf("Exit without saving? (Y/N): ");
            char c = getchar();
            clear_input();
            if (c == 'Y' || c == 'y') break;
        }
        else printf("Invalid option.\n");
    }
}

UserRole LogIn() {
    char opt[32];
    while (1) {
        printf("\n=== LogIn / Role Selection ===\n");
        printf("1. Admin\n");
        printf("2. Staff\n");
        printf("3. Guest\n");
        printf("0. Exit Program\n");
        printf("Choose role: ");

        if (!fgets(opt, sizeof(opt), stdin)) return 0;
        trim_newline(opt);
        int choice = atoi(opt);
        if (choice == 1) return ROLE_ADMIN;
        else if (choice == 2) return ROLE_STAFF;
        else if (choice == 3) return ROLE_GUEST;
        else if (choice == 0) return 0;
        else printf("Invalid choice.\n");
    }
}

int main() {
    count = read_students_from_file(students);
    printf("Student Record Management System — started. Records loaded: %d\n", count);
    while (1) {
        UserRole role = LogIn();

        if (role == 0) {
            printf("Exiting program.\n");
            break;
        } else if (role == ROLE_ADMIN) {
            AdminMenu();
        } else if (role == ROLE_STAFF) {
            StaffMenu();
        } else if (role == ROLE_GUEST) {
            GuestMenu();
        }
    }
    return 0;
}
