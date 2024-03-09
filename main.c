/*MATEI Rares-Andrei - 315CC*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pixel {
	unsigned char r, g, b;
} Pixel;

typedef struct image {
	int size;
	Pixel **data;
} Image;

typedef struct arb {
	unsigned char type;
	Pixel pixel;
	struct arb **nodes;
} *Arb, ArbNode;

typedef struct queue_node {
	Arb arb_node;
	struct queue_node *next;
} *QueueNode, SQueueNode;

typedef struct queue {
	QueueNode head, tail;
} *Queue, SQueue;

int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}

int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

void get_task(char *task, char *s) { strcpy(task, s + 1); }

void get_factor(int *factor, char *s) { sscanf(s, "%d", factor); }

void image_init(Image *image, int size)
{
	(*image).size = size;
	(*image).data = malloc(size * sizeof(Pixel *));
	int i;
	for (i = 0; i < size; i++)
		(*image).data[i] = malloc(size * sizeof(Pixel));
}

void ppm_file_read(Image *image, FILE *in)
{
	char file_type[3];
	int color_max_val, size;
	fscanf(in, "%s", file_type);
	fscanf(in, "%d%d", &size, &size);
	fscanf(in, "%d", &color_max_val);
	fgetc(in);

	image_init(image, size);

	int i;
	for (i = 0; i < (*image).size; i++) {
		fread((*image).data[i], sizeof(Pixel), (*image).size, in);
	}
}

// returneaza 1 daca e frunza in arbore (caz in care retine culorile obtinute)
// si 0 in caz contrar
int is_leaf(Image image, int line, int col, int size, int factor, Pixel *p)
{
	unsigned long long red = 0, green = 0, blue = 0, mean = 0;
	int i, j;
	for (i = line; i < line + size; i++) {
		for (j = col; j < col + size; j++) {
			red += image.data[i][j].r;
			green += image.data[i][j].g;
			blue += image.data[i][j].b;
		}
	}

	red /= size * size;
	green /= size * size;
	blue /= size * size;

	for (i = line; i < line + size; i++) {
		for (j = col; j < col + size; j++) {
			mean += (red - image.data[i][j].r) * (red - image.data[i][j].r)
				+ (green - image.data[i][j].g) * (green - image.data[i][j].g)
				+ (blue - image.data[i][j].b) * (blue - image.data[i][j].b);
		}
	}
	mean /= 3 * size * size;
	if (mean <= factor) {
		(*p).r = red;
		(*p).g = green;
		(*p).b = blue;
		return 1;
	}
	return 0;
}

Arb build_arb_from_image(Image image, int line, int col, int size, int factor)
{
	Arb arb_node = malloc(sizeof(ArbNode));
	arb_node->type = is_leaf(image, line, col, size, factor, &arb_node->pixel);
	if (!arb_node->type) {
		arb_node->nodes = malloc(4 * sizeof(ArbNode));

		size /= 2;

		arb_node->nodes[0]
			= build_arb_from_image(image, line, col, size, factor);
		arb_node->nodes[1]
			= build_arb_from_image(image, line, col + size, size, factor);
		arb_node->nodes[2] = build_arb_from_image(
			image, line + size, col + size, size, factor);
		arb_node->nodes[3]
			= build_arb_from_image(image, line + size, col, size, factor);
	}
	return arb_node;
}

// functie ce returneaza numarul de niveluri ale arborelui
int get_arb_levels_number(Arb node, int niv)
{
	if (node->type)
		return niv;
	else {
		int max_niv = 0, i;
		for (i = 0; i < 4; i++) {
			max_niv
				= max(max_niv, get_arb_levels_number(node->nodes[i], niv + 1));
		}
		return max_niv;
	}
}

// functie ce numara cate frunze sunt in arbore
int get_leaf_number(Arb node)
{
	if (node->type)
		return 1;
	int i, s = 0;
	for (i = 0; i < 4; i++)
		s += get_leaf_number(node->nodes[i]);
	return s;
}

// functie ce returneaza nivelul cel mai mic pe care se afla o frunza
int get_top_leaf_level(Arb node, int niv_t, int niv)
{
	if (node->type)
		return niv;
	else {
		int min_niv = niv_t, i;
		for (i = 0; i < 4; i++) {
			min_niv = min(
				min_niv, get_top_leaf_level(node->nodes[i], niv_t, niv + 1));
		}
		return min_niv;
	}
}

QueueNode create_queue_node(Arb arb_node)
{
	QueueNode node = malloc(sizeof(SQueueNode));
	node->arb_node = arb_node;
	node->next = NULL;
	return node;
}

void add_to_queue_tail(Queue q, Arb arb_node)
{
	if (q->tail == NULL) {
		q->head = q->tail = create_queue_node(arb_node);
	} else {
		q->tail->next = create_queue_node(arb_node);
		q->tail = q->tail->next;
	}
}

void pop_queue_head(Queue q)
{
	QueueNode aux = q->head;
	if (q->head == q->tail)
		q->head = q->tail = NULL;
	else
		q->head = q->head->next;
	free(aux);
}

void print_pixel(Pixel p, FILE *out)
{
	fwrite(&p.r, sizeof(unsigned char), 1, out);
	fwrite(&p.g, sizeof(unsigned char), 1, out);
	fwrite(&p.b, sizeof(unsigned char), 1, out);
}

void print_arb_node(Arb node, FILE *out)
{
	fwrite(&node->type, sizeof(unsigned char), 1, out);
	if (node->type) {
		print_pixel(node->pixel, out);
	}
}

void print_arb(Arb rad, FILE *out)
{
	Queue q = malloc(sizeof(SQueue));
	q->head = q->tail = NULL;
	add_to_queue_tail(q, rad);
	while (q->head) {
		print_arb_node(q->head->arb_node, out);

		if (!q->head->arb_node->type) {
			int i;
			for (i = 0; i < 4; i++)
				add_to_queue_tail(q, q->head->arb_node->nodes[i]);
		}

		pop_queue_head(q);
	}
	free(q);
}

void print_image(Image image, FILE *out)
{
	fprintf(out, "P6\n%d %d\n255\n", image.size, image.size);
	int i, j;
	for (i = 0; i < image.size; i++) {
		for (j = 0; j < image.size; j++) {
			print_pixel(image.data[i][j], out);
		}
	}
}

Arb compressed_file_read_node(FILE *in)
{
	int type;
	// daca nu a fost citit nimic inseamna ca am terminat de parcurs fisierul
	if (!fread(&type, sizeof(unsigned char), 1, in))
		return NULL;

	Arb arb_node = malloc(sizeof(ArbNode));
	arb_node->type = type;

	// daca este frunza(node_type = 1)
	if (arb_node->type) {
		fread(&arb_node->pixel, sizeof(Pixel), 1, in);
	}
	return arb_node;
}

// functie ce citeste datele dintr-un fisier comprimat si le introduce
// intr-o coada
void compressed_file_read(Queue q, int *size, FILE *in)
{
	fread(size, sizeof(int), 1, in);

	Arb arb_node;
	arb_node = compressed_file_read_node(in);
	do {
		add_to_queue_tail(q, arb_node);
		arb_node = compressed_file_read_node(in);
	} while (arb_node);
}

void build_arb_from_queue(Queue q)
{
	QueueNode current = q->head->next;

	while (current && q->head) {
		if (!q->head->arb_node->type) {
			q->head->arb_node->nodes = malloc(4 * sizeof(ArbNode));
			int i;
			for (i = 0; i < 4; i++) {
				q->head->arb_node->nodes[i] = current->arb_node;
				current = current->next;
			}
		}
		pop_queue_head(q);
	}

	// elimin restul cozii(au ramas doar frunze)
	while (q->head)
		pop_queue_head(q);
}

void build_image_from_arb(Image *image, Arb node, int line, int col, int size)
{
	if (node->type) {
		// atribui submatriciei pixelul retinut in frunza
		int i, j;
		for (i = line; i < line + size; i++) {
			for (j = col; j < col + size; j++)
				(*image).data[i][j] = node->pixel;
		}
	} else {
		// impart submatricea in 4 si continui procesul pentru fiecare
		size /= 2;
		build_image_from_arb(image, node->nodes[0], line, col, size);
		build_image_from_arb(image, node->nodes[1], line, col + size, size);
		build_image_from_arb(
			image, node->nodes[2], line + size, col + size, size);
		build_image_from_arb(image, node->nodes[3], line + size, col, size);
	}
}

void free_arbore(Arb node);

void build_image_from_compressed_file(Image *image, FILE *in)
{
	// citesc datele din fisierul comprimat si le retin intr-o coada
	int size;
	Queue q = malloc(sizeof(SQueue));
	q->head = q->tail = NULL;
	compressed_file_read(q, &size, in);

	// construiesc arborele de compresie folosind coada obtinuta,
	// primul nod din coada contine radacina arborelui
	Arb rad = q->head->arb_node;
	build_arb_from_queue(q);

	image_init(image, size);

	// construiesc imaginea folosind arborele de compresie
	build_image_from_arb(image, rad, 0, 0, (*image).size);

	free_arbore(rad);
	free(q);
}

void free_arbore(Arb node)
{
	if (!node->type) {
		int i;
		for (i = 0; i < 4; i++) {
			free_arbore(node->nodes[i]);
		}
		free(node->nodes);
	}
	free(node);
}

void free_mat(void **mat, int n)
{
	int i;
	for (i = 0; i < n; i++)
		free(mat[i]);
	free(mat);
}

int main(int argc, char *argv[])
{
	FILE *in, *out;
	in = fopen(argv[argc - 2], "rb");
	out = fopen(argv[argc - 1], "wb");

	// extrag cerinta din comanda
	char task[2];
	get_task(task, argv[1]);

	// daca am 5 argumente in comanda atunci sunt la cerinta 1 sau 2
	if (argc == 5) {
		// extrag factorul din comanda
		int factor;
		get_factor(&factor, argv[2]);

		// declar si citesc imaginea din fisierul ppm
		Image image;
		ppm_file_read(&image, in);
		fclose(in);

		// construiesc arborele
		Arb rad = build_arb_from_image(image, 0, 0, image.size, factor);

		if (!strcmp(task, "c1")) {
			int niv = get_arb_levels_number(rad, 1);

			int nr_leaves = get_leaf_number(rad);

			int top_leaf = get_top_leaf_level(rad, niv, 1);

			int max_square_size = image.size / (1 << (top_leaf - 1));

			fprintf(out, "%d\n%d\n%d\n", niv, nr_leaves, max_square_size);
		} else if (!strcmp(task, "c2")) {
			unsigned int size = image.size;
			fwrite(&size, sizeof(unsigned int), 1, out);
			print_arb(rad, out);
		}

		free_arbore(rad);
		free_mat((void **)image.data, image.size);

	} else if (!strcmp(task, "d")) {
		Image image;
		build_image_from_compressed_file(&image, in);
		fclose(in);

		print_image(image, out);

		free_mat((void **)image.data, image.size);
	}

	fclose(out);
	return 0;
}